## IP dns server

this dns server is responding with an ip address if asked for one.
the problem with a solution like zscaler is, that not all internal resources in a company are reachable through zscaler private access, extremely annoying when you are working as NetOps and most commands return an IP address.
so i wrote this dns server, which translates requests in the form of __10.1.2.3.ip.mycompany.com__ to __10.1.2.3__.  That way, the zscaler dns is getting a prober IP and translates it into the 100.64. scope and devices formerly unreachable are accessible now.

### performance
as i do not suspect as too many requests per second, that is not the main focus.  the client.c is a testing module which will be extended to benchmark the server.

### SOA, MX, et al
this is work in progress.
