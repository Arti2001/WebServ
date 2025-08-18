## Non-blocking HTTP Server (inpired by NGNIX)

WebServ is a non_blocking HTTP Server inspired by NGNIX (https://nginx.org/en/docs/). This project mostly written in c++11, although using some c++17 features(such as smart pointers).

## Prerequisites
To build and run the webserver, you need:

- Linux system (tested on Ubuntu 22.04)
- g++ with C++17 support
- make
- git
- 
## How to install? :arrow_down:

 Go to your terminal and run :
```
git clone https://github.com/Arti2001/WebServ.git webserv
```

## How to compile?
⚠️ Note: This webserver is designed for Linux systems only.  
It will **not compile(run) on macOS** due to OS-level differences in networking and system calls.



* Go to the installed repo:
```
cd webserv
```
* Then run:
```
make
```
### Congrats the program is compiled now :wink:

## How to Run?
So, at this point my friend, you should know what a configuration file is https://www.digitalocean.com/community/tutorials/understanding-the-nginx-configuration-file-structure-and-configuration-contexts

* Our program takes a configuration file as an argument.
```
./webserv example.conf
```

⚠️ Note: If no arguments  were provided the program will use a **default configuration file**, which has restricted abilities.

## More about a configuration file.

The webserver uses a custom configuration file inspired by NGINX.
Syntax is similar to NGINX, but not fully the same.


### Configuration file syntax
* Only server and location blocks are supported.

  ```
    server {
  
      ...
  
      location /{
        ...
      }
  
    }

    server {
  
        ...
  
    }
  ```

    ### Directives
 
    **Server** Block Directives
  
    | Directive              | Description                                     | Example                            |
    | ---------------------- | ----------------------------------------------- | ---------------------------------- |
    | `listen`               | IP address the server binds to `ip : port`      | `host 0.0.0.0:8080`                |
    | `server_name`          | List of server names (used for virtual hosting) | `server_name localhost;`           |
    | `root`                 | Root directory for this server                  | `root website/;`                   |
    | `index`                | Default index files to serve                    | `index index.html;`                |
    | `autoindex`            | Enable/disable directory listing (`on` / `off`) | `autoindex off;`                   |
    | `client_max_body_size` | Maximum allowed request body size (bytes)       | `client_max_body_size 2G;`    |
    | `error_page`           | Map of error codes to custom error pages        | `error_page 404 /errors/404.html;` |



  **Location** Block Directives
    
  | Directive              | Description                                                                 | Example                             |
  | ---------------------- | --------------------------------------------------------------------------- | ----------------------------------- |
  | `path`                 | Path prefix this location applies to                                              | `location /images { ... }`          |
  | `upload_path`          | Directory where uploaded files will be stored                                     | `upload_path website/uploads/;`     |
  | `root`                 | Root directory for this location (fallback to server root if not specified)       | `root /var/www/images;`             |
  | `index`                | Index files for this location (fallback to server index)                          | `index index.html;`                 |
  | `autoindex`            | Enable/disable directory listing (off, on)                                        |`autoindex on;`                      |
  | `client_max_body_size` | Maximum allowed request body size (overrides server value)                        | `client_max_body_size 2097152;`     |
  | `allowed_methods`      | Set of allowed HTTP methods (`GET`, `POST`, `DELETE`)(fallback to server methods) | `allowed_methods GET POST;`         |
  | `allowed_cgi`          | Map of file extensions to CGI scripts                                             | `allowed_cgi .py=/usr/bin/python3;` |
  | `return`               | Return directive for redirects or short responses                                 | `return 301 /new-location;`         |
  | `error_page`           | Custom error pages for this location (fallback to server error pages)             | `error_page 403 /errors/403.html;`  |


⚠️ **Notes**:

* Location blocks inherit defaults from the server block unless overridden.
* All directives must end with ; .
* You can specify the size in bytes, or append M/m for megabytes and G/g for gigabytes
* All bracket must be closed!


### Example

```
  server {

    listen localhost:8080               # IP address to bind : TCP port
    server_name myserver.com;           # Virtual host names
    root website/;                      # Root directory
    index index.html;                   # Default index files
    autoindex off;                      # Directory listing off
    client_max_body_size 1G;            # Max body size (1G = 1 gigabyte)
    error_page 404 /errors/404.html;    # Custom error pages
    error_page 500 /errors/500.html;

    location / {
        path /;                         # Path prefix for this location
        root website/;                  # Root directory (fallback to server root)
        index index.html;               # Index files (fallback to server index)
        autoindex 0;                     # Directory listing (0=off, 1=on)
        client_max_body_size 1G;        # Max body size (overrides server block)
        allowed_methods GET;            # Allowed HTTP methods
        allowed_cgi .py=/usr/bin/python3; # Map extensions to CGI scripts
        upload_path website/uploads/;   # Directory for uploads
        return 0 "";                     # No redirect (use 301/302 for redirects)
        error_page 403 /errors/403.html;
    }

    location /images {
        path /images;
        root website/images/;
        index index.html;
        autoindex 1;                     # Directory listing on
        client_max_body_size 500M;       # 500 megabytes
        allowed_methods GET POST;
        allowed_cgi .php=/usr/bin/php-cgi;
        upload_path website/uploads/images/;
        return 301 /new-location;        # Redirect example
        error_page 404 /errors/404.html;
    }
}
```


