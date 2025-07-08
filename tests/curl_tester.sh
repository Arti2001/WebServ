echo "Testing curl commands against the web server..."
echo "Check the location in cases tests need to be adjusted. Current port is 8071."
curl http://localhost:8071/ # update the port if needed.
echo "Error code should be 200 OK"
curl -X POST --data "somedatahere" http://localhost:8071/ # update the port if needed
echo "Error code should be 405 Method Not Allowed"
curl -X PUT http://localhost:8071/ # update the port if needed PUT method has content-lengh header, so we need to add a check for status code.
echo "Error code should be 400 Bad Request"
curl -X DELETE http://localhost:8071/ # update the port if needed
echo "Error code should be 405 Method Not Allowed"
curl -H "Host: lol.com" http://localhost:8071/ # update the port if needed
echo "Error code should be 404 Not Found"
curl -H "Host: testServer.com" http://localhost:8071/ # update the port if needed
echo "Error code should be 200 OK"
curl http://localhost:8071/invalid1/ # update the port if needed
echo "Error code should be 404 Not Found"
curl http://localhost:8080/ # update the port if needed
echo "Error code should be 404 Not Found"
curl http://localhost:8081/ # update the port if needed
curl -X POST --data "somedatahere" http://localhost:8071/simple/ # update the port if needed
echo "Error code should be 200 OK"
curl -X DELETE http://localhost:8071/simple/blackhole.png # update the port if needed
echo "Error code should be 200 OK"
curl http://localhost:8071/redirect/ # update the port if needed
echo "Error code should be 301 Moved Permanently"




