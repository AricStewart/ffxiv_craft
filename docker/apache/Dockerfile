FROM httpd:2.4.33-alpine
RUN apk update; \
    apk upgrade;
RUN echo "LoadModule rewrite_module modules/mod_rewrite.so" \
    > /usr/local/apache2/conf/rewrite.conf
RUN echo "Include /usr/local/apache2/conf/rewrite.conf" \
    >> /usr/local/apache2/conf/httpd.conf
COPY demo.apache.conf /usr/local/apache2/conf/demo.apache.conf
RUN echo "Include /usr/local/apache2/conf/demo.apache.conf" \
    >> /usr/local/apache2/conf/httpd.conf
