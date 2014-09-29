BubbleQA clones a Drupal project into a directory so it can be tested independently.

INSTALLATION
============

1. Setup up a master site. For example, at /var/www/mysite-master

2. Make settings.php at /var/www/mysite-master writable.

3. Create a directory for bubbles to be built. For example, at /var/www/bubble-sites

4. Enable Apache's vhost_alias module with the following command:

sudo a2enmod vhost_alias_module

5. Create a Virtual host for the Bubble sites. You can adjust this template:

# /etc/apache2/sites-available/bubble-sites.conf
<VirtualHost *:80>
  ServerName bubble.local
  ServerAlias *.bubble.local
  VirtualDocumentRoot /home/juampy/projects/bubble-sites/%1
  RewriteEngine On
  LogLevel debug
  <Directory /home/juampy/projects/bubble-sites>
    Options +FollowSymLinks
    AllowOverride All
    order allow,deny
    allow from all
  </Directory>

  CustomLog /var/log/apache2/bubble_sites_access.log combined
  ErrorLog /var/log/apache2/bubble_sites_error.log
</VirtualHost>
