pandoc \
--pdf-engine=xelatex \
--number-sections \
-Vsecnumdepth=3 \
-Vmargin-left=4cm \
-Vmargin-right=4cm \
--from markdown+smart \
final.md -o final.pdf \
&& \
open final.pdf
