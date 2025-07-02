#!/bin/sh
if [ ! -f "/var/www/wp-config.php" ]; then
cat << EOF > /var/www/wp-config.php
<?php
define( 'DB_NAME', '${DB_NAME}' );
define( 'DB_USER', '${DB_USER}' );
define( 'DB_PASSWORD', '${DB_PASS}' );
define( 'DB_HOST', 'mariadb' );
define( 'DB_CHARSET', 'utf8' );
define( 'DB_COLLATE', '' );
define('FS_METHOD','direct');
\$table_prefix = 'wp_';
define( 'WP_DEBUG', false );
if ( ! defined( 'ABSPATH' ) ) {
define( 'ABSPATH', __DIR__ . '/' );}
require_once ABSPATH . 'wp-settings.php';
EOF
fi

# echo "Creating user '${DB_USER}' in db"
# cat << EOF | mysql -u ${DB_USER} -p${DB_PASS} -h mariadb ${DB_NAME}
# INSERT INTO wp_users (user_login, user_pass, user_nicename, user_email, user_registered, user_status)
# VALUES ('${DB_USER}', MD5('${DB_PASS}'), '${DB_USER}', 'acme@acme.com', NOW(), 0);

# INSERT INTO wp_usermeta (user_id, meta_key, meta_value)
# VALUES (LAST_INSERT_ID(), 'wp_capabilities', 'a:1:{s:10:"subscriber";b:1;}'),
#        (LAST_INSERT_ID(), 'wp_user_level', '0');
# EOF