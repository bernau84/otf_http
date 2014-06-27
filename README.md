otf_http
========

on-the-fly http parser

* C++, no dynamic allocation
* direct mime/http parser works on proprietar circular buffer
* no data copying, headers and body is marked directly in the input stream
* stream can be updated pat by part, parser act as state machine
* multipart support 
* end of http is determined either by http length or multipart boundary
* fully commented, basic tests inluded
* 
