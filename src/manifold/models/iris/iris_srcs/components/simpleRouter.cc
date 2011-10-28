#ifndef  SIMPLEROUTER_CC_INC
#define  SIMPLEROUTER_CC_INC

#include	"simpleRouter.h"


// memory request state noc string
const char* mem_req_noc_type_name[MAX_NOC_STATE] = {
  "NOC_FILL",
  "NOC_NEW",
  "NOC_LAST",
};


SimpleRouter::SimpleRouter ( macsim_c* simBase )
{
    m_simBase = simBase;
}

SimpleRouter::~SimpleRouter ()
{
	for ( uint i=0; i<ports; i++)
    {
        delete &in_buffers[i];
        delete &decoders[i];
        downstream_credits[i].clear();
        delete &vca;
        delete &swa;
    }
    input_buffer_state.clear();
}

void
SimpleRouter::parse_config(map<string,string>& p)
{
    ports = 3;
    vcs = 4;
    credits = 1;
    rc_method = RING_ROUTING;
    no_nodes = 16;
    grid_size = no_nodes;

    map<string,string>::iterator it;
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());
    it = p.find("grid_size");
    if ( it != p.end())
        grid_size= atoi((it->second).c_str());
    it = p.find("no_ports");
    if ( it != p.end())
        ports = atoi((it->second).c_str());
    it = p.find("credits");
    if ( it != p.end())
        credits = atoi((it->second).c_str());
    it = p.find("no_vcs");
    if ( it != p.end())
        vcs = atoi((it->second).c_str());
    it = p.find("rc_method");
    if ( it != p.end())
    {
        if(it->second.compare("TWONODE_ROUTING") == 0)
            rc_method = TWONODE_ROUTING;
        if(it->second.compare("RING_ROUTING") == 0)
            rc_method = RING_ROUTING;
        if(it->second.compare("TORUS_ROUTING") == 0)
            rc_method = TORUS_ROUTING;
        if(it->second.compare("XY_ROUTING") == 0)
            rc_method = XY_ROUTING;
    }


    return;
}

void
SimpleRouter::init( void )
{

    in_buffers.resize(ports);
    decoders.resize(ports);
    input_buffer_state.resize(ports*vcs);
    vca.setup( ports, vcs );
    swa.resize(ports,vcs);
    downstream_credits.resize(ports);
    stat_pp_avg_buff.resize(ports);
    stat_pp_packets_out.resize(ports);
    stat_pp_pkt_out_cy.resize(ports);
    stat_pp_avg_lat.resize(ports);

    for(uint i=0; i<ports; i++)
    {
        decoders[i].grid_size = grid_size;
        decoders[i].node_id = node_id;
        decoders[i].address = 0;
        decoders[i].rc_method = rc_method;
        decoders[i].no_nodes = no_nodes;
        decoders[i].init();
        stat_pp_avg_buff[i].resize(vcs);
        stat_pp_packets_out[i].resize(vcs);
        stat_pp_pkt_out_cy[i].resize(vcs);
        stat_pp_avg_lat[i].resize(vcs);
    }

    // useful when debugging the swa and vca... but not needed for operation
    swa.node_ip = node_id; 
    vca.node_ip = node_id; 

    for(uint i=0; i<ports; i++)
    {
        downstream_credits[i].resize(vcs);
        in_buffers[i].resize( vcs, credits);
        decoders[i].resize( vcs );
    }

    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            downstream_credits[i][j] = credits;
            input_buffer_state[i*vcs+j].pipe_stage = INVALID;
            input_buffer_state[i*vcs+j].clear_message = true;
            input_buffer_state[i*vcs+j].address = -1 ;
            input_buffer_state[i*vcs+j].input_port = -1;
            input_buffer_state[i*vcs+j].input_channel = -1;
            input_buffer_state[i*vcs+j].output_port = -1;
            input_buffer_state[i*vcs+j].output_channel = -1;
        }

    reset_stats();

    return ;
}

void
SimpleRouter::reset_stats ( void )
{
    // stats init
    stat_avg_buff = 0;
    stat_packets_out = 0;
    stat_packets_in = 0;
    stat_flits_out = 0;
    stat_flits_in = 0;
    avg_router_latency = 0;
    stat_last_flit_out_cycle= 0;
    stat_3d_packets_out = 0;
    stat_2d_packets_out = 0;
    stat_3d_flits_out = 0;
    stat_2d_flits_out = 0;

    ib_cycles = 0;
    vca_cycles = 0;
    sa_cycles = 0;
    st_cycles = 0;

    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            stat_pp_avg_buff[i][j]=0;
            stat_pp_packets_out[i][j]=0;
            stat_pp_pkt_out_cy[i][j]=0;
            stat_pp_avg_lat[i][j]=0;
        }

    return ;
}		/* -----  end of method SimpleRouter::reset_stats  ----- */

const char *mem_state_copy[] = {
  "MEM_INV",
  "MEM_NEW",
  "MEM_MERGED",
  "MEM_OUT_NEW",
  "MEM_OUT_FILL",
  "MEM_OUT_WB",
  "MEM_FILL_NEW",
  "MEM_FILL_WAIT_DONE",
  "MEM_FILL_WAIT_FILL",
  "MEM_DRAM_START",
  "MEM_DRAM_CMD",
  "MEM_DRAM_DATA",
  "MEM_DRAM_DONE",
  "MEM_NOC_START",
  "MEM_NOC_DONE",
};
void
SimpleRouter::handle_link_arrival( int port, LinkData* data )
{
    switch ( data->type ) {
        case FLIT:	
            {
                //_DBG("FLIT came in inport is %d from source %d", port, data->src);
                /* Stats update */
                stat_flits_in++;
                ib_cycles++;
                
                if ( data->f->type == TAIL ) stat_packets_in++;

                uint inport = port%ports;
                //push the flit into the buffer. Init for buffer state done
                //inside do_input_buffering
                in_buffers[inport].change_push_channel( data->vc );
                in_buffers[inport].push(data->f);
                if ( data->f->type == HEAD )
                {
                #if 1
                	cout << manifold::kernel::Manifold::NowTicks() << " IRIS pkt " 
                		<< ((HeadFlit*)data->f)->req->m_id 
                		<< " arrived @ node " << node_id 
                		<< " bound for " << ((HeadFlit*)data->f)->dst_node 
                		<< " mem state " << mem_state_copy[((HeadFlit*)data->f)->req->m_state]
                		<< " msg type " << mem_req_noc_type_name[((HeadFlit*)data->f)->req->m_msg_type] << "\n";
                		//<< " vc: " << data->vc << "\n";
                    //for stats collection/visualization
//                    cout << manifold::kernel::Manifold::NowTicks() << "," 
//                        << ((HeadFlit*)data->f)->req->m_id << ","
//                        << mem_state_copy[((HeadFlit*)data->f)->req->m_state] << ","
//                        << mem_req_noc_type_name[((HeadFlit*)data->f)->req->m_msg_type] << ","
//                        << node_id << "," << ((HeadFlit*)data->f)->dst_node << ","
//                        << "R" << "," << "0" << "\n";
                #else 
                  //for debugging only!
                  int stage = -1;
                  static const int stage_state[3][3] = {    //transposed array for column major order
                    {7, 0, -1},                             //no idea why it's not row major order
                    { 6, 1, 2}, 
                    {-1, 4, 3}
                  };
                  
                  //si debug
                  if(manifold::kernel::Manifold::NowTicks() == 6000 )
                    cout << "what is stuck?" << endl;
                  
                  stage = stage_state[node_id][((HeadFlit*)data->f)->dst_node];
                  stage += (stage == 1 && ((HeadFlit*)data->f)->req->m_state == 12) ? 4 : 0;
                  
                  cout << manifold::kernel::Manifold::NowTicks() << ", "
                    << ((HeadFlit*)data->f)->req->m_id << ", "
                    << stage << ", "
                    << node_id << ", "
                		<< ((HeadFlit*)data->f)->dst_node << ", "
                    << mem_state_copy[((HeadFlit*)data->f)->req->m_state] << ", \n";
                #endif
                    do_input_buffering(static_cast<HeadFlit*>(data->f), inport, data->vc);
                }
                else
                    decoders[inport].push(data->f, data->vc);

                break;
            }
        case CREDIT:	
            {
                /*  Update credit information for downstream buffer
                 *  corresponding to port and vc */
                uint inport = port%ports;
                downstream_credits[inport][data->vc]++;

                break;
            }

        default:	
            cerr << " ERROR: SimpleRouter::handle_link_arrival" << endl;
            break;
    }				/* -----  end switch  ----- */

    delete data;
    return ;
}

void
SimpleRouter::do_input_buffering(HeadFlit* hf, uint inport, uint invc)
{
    input_buffer_state[inport*vcs+invc].input_port = inport;
    input_buffer_state[inport*vcs+invc].input_channel = invc;
    input_buffer_state[inport*vcs+invc].address = hf->address;
    input_buffer_state[inport*vcs+invc].destination = hf->dst_node;
    input_buffer_state[inport*vcs+invc].src_node= hf->src_node;
    input_buffer_state[inport*vcs+invc].pipe_stage = FULL;
    input_buffer_state[inport*vcs+invc].mclass = hf->mclass;
    input_buffer_state[inport*vcs+invc].pkt_length = hf->pkt_length;
    input_buffer_state[inport*vcs+invc].pkt_arrival_time = manifold::kernel::Manifold::NowTicks();
    input_buffer_state[inport*vcs+invc].clear_message = false;
    input_buffer_state[inport*vcs+invc].sa_head_done = false;
#ifdef _DEBUG
    input_buffer_state[inport*vcs+invc].fid= hf->flit_id;
#endif

    //Routing
    decoders[inport].push(hf,invc);
    input_buffer_state[inport*vcs+invc].possible_oports.clear();
    input_buffer_state[inport*vcs+invc].possible_ovcs.clear();
    uint rc_port = decoders[inport].get_output_port(invc);
    input_buffer_state[inport*vcs+invc].possible_oports.push_back(rc_port);
    uint rc_vc = decoders[inport].get_virtual_channel(invc);
    input_buffer_state[inport*vcs+invc].possible_ovcs.push_back(rc_vc);

    assert ( input_buffer_state[inport*vcs+invc].possible_oports.size() != 0);
    assert ( input_buffer_state[inport*vcs+invc].possible_ovcs.size() != 0);


}

void SimpleRouter::do_vc_allocation()
{
    if(!vca.is_empty())
    {
        vca.pick_winner();
        vca_cycles++;
    }

    for( uint i=0; i<ports; i++)
        for ( uint ai=0; ai<vca.current_winners[i].size(); ai++)
        {
            VCA_unit winner = vca.current_winners[i][ai];
            uint ip = winner.in_port;
            uint ic = winner.in_vc;
            uint op = winner.out_port;
            uint oc = winner.out_vc;
//_DBG(" vca winner %d=%d %d", op,oc, vca.current_winners[i]);

            uint msgid = ip*vcs+ic;
            if ( input_buffer_state[msgid].pipe_stage == VCA_REQUESTED ) // && downstream_credits[op][oc]==credits)
            {
                input_buffer_state[msgid].output_port = op;
                input_buffer_state[msgid].output_channel= oc;
                input_buffer_state[msgid].pipe_stage = VCA_COMPLETE;
                // if requesting multiple outports make sure to cancel them as
                // pkt will no longer be VCA_REQUESTED
            }

        }

    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == VCA_COMPLETE)
        {
            uint ip = input_buffer_state[i].input_port;
            uint ic = input_buffer_state[i].input_channel;
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;

            if(in_buffers[ip].get_occupancy(ic))
            {
                swa.request(op, oc, ip, ic);
                input_buffer_state[i].pipe_stage = SWA_REQUESTED;
                                _DBG(" SWA req %d %d op %d %d", ip, ic, op, oc);
            }
        }

    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == FULL )
            input_buffer_state[i].pipe_stage = IB;

    /* Request VCA */
    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == IB )
        {
            uint ip = input_buffer_state[i].input_port;
            uint ic = input_buffer_state[i].input_channel;
            uint op = input_buffer_state[i].possible_oports[0];
            uint oc = -1;
            for ( uint ab=0; ab<input_buffer_state[i].possible_ovcs.size();ab++)
            {
                oc = input_buffer_state[i].possible_ovcs[ab];
                //                if( downstream_credits[op][oc] == credits &&
                if ( !vca.is_requested(op, oc, ip, ic) ) 
                {
_DBG(" Req vca for msg ip%d %d", ip,ic);
                    input_buffer_state[i].pipe_stage = VCA_REQUESTED;
                    vca.request(op,oc,ip,ic);
                    //break;      /* allow just one */
                }
            }
        }
}

void 
SimpleRouter::do_switch_traversal()
{
    /* Switch traversal */
    for( uint i=0; i<ports*vcs; i++)
        if( input_buffer_state[i].pipe_stage == SW_TRAVERSAL)
        {
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;
            uint ip = input_buffer_state[i].input_port;
            uint ic= input_buffer_state[i].input_channel;
            if( in_buffers[ip].get_occupancy(ic)> 0
                && downstream_credits[op][oc]>0 )
            {
                in_buffers[ip].change_pull_channel(ic);
                Flit* f = in_buffers[ip].pull();
                f->virtual_channel = oc;

                //If the tail is going out then its cleared later
                swa.request(op, oc, ip, ic);
                input_buffer_state[i].pipe_stage = SWA_REQUESTED;

                if( f->type == TAIL || f->pkt_length == 1)
                {
                    /* Update packet stats */
                    uint lat = manifold::kernel::Manifold::NowTicks()-
                        input_buffer_state[i].pkt_arrival_time;
                    avg_router_latency += lat;
                    stat_packets_out++;
                    stat_pp_packets_out[op][oc]++;
                    stat_pp_pkt_out_cy[op][oc] = manifold::kernel::Manifold::NowTicks();
                    stat_pp_avg_lat[op][oc] += lat;

                    if  ( op != 5 && op != 0)  stat_2d_packets_out++; else if ( op == 5) stat_3d_packets_out++;
//                    printf("Time%lld node:%d stat_3d_packets_out %lld stat_2d_packets_out:%lld op%d addr:%llx\n",
//                           manifold::kernel::Manifold::NowTicks(), node_id, stat_3d_packets_out,stat_2d_packets_out,op, input_buffer_state[i].address);
                    
                    input_buffer_state[i].clear_message = true;
                    input_buffer_state[i].pipe_stage = EMPTY;
                    input_buffer_state[i].input_port = -1;
                    input_buffer_state[i].input_channel = -1;
                    input_buffer_state[i].output_port = -1;
                    input_buffer_state[i].output_channel = -1;
                    vca.clear_winner(op, oc, ip, ic);
                    swa.clear_requestor(op, ip,oc);
                    input_buffer_state[i].possible_oports.clear();
                    input_buffer_state[i].possible_ovcs.clear();
                }

                stat_flits_out++;
                st_cycles++;
                if  ( op != 5 && op != 0)  stat_2d_flits_out++; else if ( op == 5) stat_3d_flits_out++;

                LinkData* ld = new LinkData();
                ld->type = FLIT;
                ld->src = this->GetComponentId();
                f->virtual_channel = oc;
                ld->f = f;
                ld->vc = oc;

                //Not using send then identify between router and interface
                //Use manifold send here to push the ld obj out

                Send( data_outports.at(op), ld);    //schedule cannot be used here as the component is not on the same LP

                /*! \brief Send a credit back and update buffer state for the
                 * downstream router buffer */
                downstream_credits[op][oc]--;

                LinkData* ldc = new LinkData();
                ldc->type = CREDIT;
                ldc->src = this->GetComponentId();
                ldc->vc = ic;
_DBG(" FLIT out %d=%d", op,oc);

                Send(signal_outports.at(ip),ldc);   /* Int not on same LP */
                stat_last_flit_out_cycle= manifold::kernel::Manifold::NowTicks();

            }
            else
            {
                input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                swa.clear_requestor(op, ip,oc);
            }
        }
}

void 
SimpleRouter::do_switch_allocation()
{
    /* Switch Allocation */
    for( uint i=0; i<ports*vcs; i++)
        if( input_buffer_state[i].pipe_stage == SWA_REQUESTED)
            if ( !swa.is_empty())
            {
                sa_cycles++;    // stat
                
                uint op = -1, oc = -1;
                SA_unit sa_winner;
                uint ip = input_buffer_state[i].input_port;
                uint ic = input_buffer_state[i].input_channel;
                op = input_buffer_state[i].output_port;
                oc= input_buffer_state[i].output_channel;
                sa_winner = swa.pick_winner(op);

                bool alloc_done = false;
                if(input_buffer_state[i].sa_head_done)
                {
                    if( sa_winner.port == ip && sa_winner.ch == ic
                        && in_buffers[ip].get_occupancy(ic) > 0
                        && downstream_credits[op][oc]>0 )
                    {
                        input_buffer_state[i].pipe_stage = SW_TRAVERSAL;
                        alloc_done = true;
                    }
                }
                else
                {
                    if( sa_winner.port == ip && sa_winner.ch == ic
                        //                        && in_buffers[ip].get_occupancy(ic) > 0
                        && downstream_credits[op][oc]==credits )
                    {
                        input_buffer_state[i].sa_head_done = true;
                        input_buffer_state[i].pipe_stage = SW_TRAVERSAL;
                        alloc_done = true;
#if 0                       
                        cout << "IRIS packet " << ((HeadFlit*)f)->req->m_id << " @ node "
		                    << node_id << " in oport " << data_outports.at(op)
		                    << "\n";
#endif
                    }

                }
                if ( !alloc_done )
                {
                    input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                    swa.clear_requestor(op, ip,oc);
                }

            }
}
void
SimpleRouter::tick ( void )
{
    return ;
}

extern void dump_state_at_deadlock(macsim_c* simBase);
void
SimpleRouter::tock ( void )
{
    /* 
     * This can be used to check if a message does not move for a large
     * interval. ( eg. interval of arrival_time + 100 )
     *
     for ( uint i=0; i<input_buffer_state.size(); i++)
     if (input_buffer_state[i].pipe_stage != EMPTY
     && input_buffer_state[i].pipe_stage != INVALID
     && input_buffer_state[i].pkt_arrival_time+100 < manifold::kernel::Manifold::NowTicks())
     {
     fprintf(stderr,"\n\nDeadlock at Router %d node %d Msg id %d Fid%d", GetComponentId(), node_id, i, input_buffer_state[i].fid);
     dump_state_at_deadlock(m_simBase);
     }
     /* */
    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            stat_pp_avg_buff[i][j] += in_buffers[i].get_occupancy(j);
            stat_avg_buff += in_buffers[i].get_occupancy(j);
        }
        
    do_switch_traversal();
    do_switch_allocation();
    do_vc_allocation();
    return ;
}

std::string
SimpleRouter::print_stats ( void ) const
{
    std::stringstream str;
    str 
        << "\n SimpleRouter[" << node_id << "] packets_in: " << stat_packets_in
        << "\n SimpleRouter[" << node_id << "] packets_out: " << stat_packets_out
        << "\n SimpleRouter[" << node_id << "] flits_in: " << stat_flits_in
        << "\n SimpleRouter[" << node_id << "] flits_out: " << stat_flits_out
        << "\n SimpleRouter[" << node_id << "] avg_router_latency: " << (avg_router_latency+0.0)/stat_packets_out
        << "\n SimpleRouter[" << node_id << "] last_pkt_out_cy: " << stat_last_flit_out_cycle
        << "\n SimpleRouter[" << node_id << "] stat_3d_packets_out: " << stat_3d_packets_out 
        << "\n SimpleRouter[" << node_id << "] stat_2d_packets_out: " << stat_2d_packets_out 
        << "\n SimpleRouter[" << node_id << "] stat_3d_flits_out: " << stat_3d_flits_out 
        << "\n SimpleRouter[" << node_id << "] stat_2d_flits_out: " << stat_2d_flits_out 
        << "\n SimpleRouter[" << node_id << "] inj_nodes_packets_out: " << stat_packets_out - ( stat_2d_packets_out + stat_3d_packets_out )
        << "\n SimpleRouter[" << node_id << "] local_remote_ratio: " << stat_3d_packets_out*1.0/stat_2d_packets_out 
        << "\n SimpleRouter[" << node_id << "] stat_3d_flits_out: " << stat_3d_flits_out
        << "\n SimpleRouter[" << node_id << "] stat_2d_flits_out: " << stat_2d_flits_out
        << "\n SimpleRouter[" << node_id << "] ib_cycles: " << ib_cycles
        << "\n SimpleRouter[" << node_id << "] sa_cycles: " << sa_cycles 
        << "\n SimpleRouter[" << node_id << "] vca_cycles: " << vca_cycles
        << "\n SimpleRouter[" << node_id << "] st_cycles: " << st_cycles
        ;
    str << "\n SimpleRouter[" << node_id << "] per_port_avg_buff: ";
    
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            str << stat_pp_avg_buff[i][j]*1.0/manifold::kernel::Manifold::NowTicks() << " ";

    str << "\n SimpleRouter[" << node_id << "] per_port_pkts_out: ";
        for ( uint i=0; i<ports; i++)
            for ( uint j=0; j<vcs; j++)
                str << stat_pp_packets_out[i][j] << " ";

        str << "\n SimpleRouter[" << node_id << "] per_port_avg_lat: ";
        for ( uint i=0; i<ports; i++)
            for ( uint j=0; j<vcs; j++)
                if ( stat_pp_packets_out[i][j] ) 
                    str  << (stat_pp_avg_lat[i][j]+0.0)/stat_pp_packets_out[i][j] << " ";
                else
                    str  << "0 ";

        str << "\n SimpleRouter[" << node_id << "] per_port_last_pkt_out_cy: ";
        for ( uint i=0; i<ports; i++)
            for ( uint j=0; j<vcs; j++)
                str << stat_pp_pkt_out_cy[i][j] << " ";

        if ( ( stat_packets_in - stat_packets_out ) > ports*vcs ) str << "\n ERROR pkts_in-pkts_out > buffering available";
        str << std::endl;

        return str.str();
} /* ----- end of function GenericPktGen::toString ----- */

#endif   /* ----- #ifndef SIMPLEROUTER_CC_INC  ----- */

