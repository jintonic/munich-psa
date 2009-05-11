latex:
	export TEXMFCNF=/remote/pclg-05/jingliu/usr/share/texmf/web2c
	latex memo
ps:
	export TEXMFCNF=/remote/pclg-05/jingliu/usr/share/texmf/web2c
	dvips memo.dvi -o memo.ps
pdf:
	export TEXMFCNF=/remote/pclg-05/jingliu/usr/share/texmf/web2c
	pdflatex -shell-escape memo
clean:
	rm -f memo.pdf memo.dvi *.bak *.pstex* *.aux *.toc *.log *.out *.ps
