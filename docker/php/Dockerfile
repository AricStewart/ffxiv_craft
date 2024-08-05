FROM php:8.3.0-fpm-alpine
RUN apk update \
    && apk upgrade \
    && apk add git bash msmtp freetype libpng libjpeg-turbo freetype-dev libpng-dev libjpeg-turbo-dev linux-headers;
RUN docker-php-ext-install mysqli pdo_mysql
RUN docker-php-ext-install gd
RUN apk add $PHPIZE_DEPS \
    && pecl install xdebug \
    && docker-php-ext-enable xdebug
COPY msmtprc /.msmtprc
COPY php.ini /usr/local/etc/php/php.ini
