pandoc final.md > body.html
cat head.html body.html tail.html > final.html
wkhtmltopdf \
-q \
--page-size A4 \
--dpi 300 \
--print-media-type \
final.html final.pdf
