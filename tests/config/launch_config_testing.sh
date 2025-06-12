make 

echo "Testing error cases in location blocks\n"
echo "Duplicate root directive\n"
./test_config ../../config-files/error/location/dup-root.conf
echo "\nInvalid http method\n"
./test_config ../../config-files/error/location/invalid-allow_methods.conf
echo "\nInvalid CGI extension\n"
./test_config ../../config-files/error/location/invalid-cgi_extension.conf
echo "\nUnknown directive\n"
./test_config ../../config-files/error/location/unknown-directive.conf
echo "\nDuplicate upload path\n"
./test_config ../../config-files/error/location/upload_path.conf

make fclean