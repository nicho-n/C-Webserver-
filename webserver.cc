//Nick Nowak 
//found in my gists from two years ago

#include <iostream>             // cout, cerr, etc
#include <stdio.h>              // perror
#include <string.h>             // bcopy
#include <netinet/in.h>         // struct sockaddr_in
#include <unistd.h>             // read, write, etc
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <sstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <fstream>
#include <time.h>

using namespace std;
string line;
const int BUFSIZE=10240;

main(int argc, char *argv[]) {
        int s;                  // socket descriptor
        int len;                // length of reveived data
        char buf[BUFSIZE];      // buffer in which to read
        int ret;                // return code from various system calls
        string ext;
        string contenttype;
        string command; string filename; string protocol; string referer = ""; string useragent; string word;
        s = MakeServerSocket(argv[1]);

        while (1) {
                struct sockaddr_in sa;
                int sa_len = sizeof(sa);
                int fd = accept(s, (struct sockaddr *)&sa, (unsigned int *)&sa_len);
                assert(fd != -1);
                memset(buf, 0, BUFSIZE);

                // Read a bit of data
                int len = read(fd, buf, BUFSIZE);
                cout << "------------Read says:" << len << "--------\n";
                buf[len] = 0;

                //Get the time for log file
                time_t curtime; time(&curtime);
                cout << ctime(&curtime);

                //Get command, file name, protocol
                string HTTPrequest = buf; istringstream stream(HTTPrequest);
                stream >> command; stream >> filename; stream >> protocol;

                //Get referer if it exists
                if (strstr(HTTPrequest.c_str(), "Referer:")){
                        while (word != "Referer:")
                                stream >> word;
                        stream >> referer;
                }

                //Remove slash in file name
                if (filename.length() > 1)
                        filename.erase(0, 1);
		
	        //Attempt to open file
                int in = open(filename.c_str(), ios::out);

                //Ignore favicon
                if (filename == "favicon.ico") goto Writelog;

                //Change file to 404 page if it does not exist
                if (in == -1){
                        close(in); filename = "404error.html";
                        in = open(filename.c_str(), ios::out);
                }

                //Change file to 403 page if client is IE
                if (strstr(HTTPrequest.c_str(), "Trident")){
                        close(in); filename = "403error.html";
                        in = open(filename.c_str(), ios::out);
                }

                //Get file extension if it exists
                if (filename.find('.'))
                        ext = strrchr(filename.c_str(), '.');

                //Write metadata if protocol is not 0.9
                if (protocol != "HTTP/0.9"){
                        string success = protocol + " 200 OK \r\n";
                        if (ext == ".txt" || ext == ".html" || ext == ".cc")
                                contenttype = "Content-type: text/html \r\n";
                        if (ext == ".gif")
                                contenttype = "Content-type: image/gif \r\n";
                        if (ext == ".jpg" || ext == ".jpeg")
                                contenttype = "Content-type: image/jpeg \r\n";

                        string metadata = success + contenttype + "\r\n";
                        int r = write(fd, metadata.c_str(), metadata.length());
                }

                //Reset buffer and write file to socket
                memset(buf, 0, BUFSIZE);
                if (ext==".html" || ext == ".cc" || ext==".jpg" || ext==".jpeg" || ext==".gif" || ext==".txt"){
		            int len = read(in, buf, BUFSIZE);
                        while (1){
                                int error = write(fd, buf, BUFSIZE);
                                len = read(in, buf, BUFSIZE);
                                if (len == 0) break;
                        }
			
                writelog(200);
		continue;
           }    
}
