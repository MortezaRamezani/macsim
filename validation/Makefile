RM = /bin/rm -rf


all: 
	pdflatex -output-format=pdf -jobname=paper main; 
	bibtex paper; 
	pdflatex -output-format=pdf -jobname=paper main; 
	pdflatex -output-format=pdf -jobname=paper main; 

refs:
	ln -s ../bib/refs.bib;

clean: 
	$(RM) *.log *.aux *.blg *.bbl *.dvi paper.ps paper.pdf
