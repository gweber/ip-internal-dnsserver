## zscaler dns server

this dns server is responding with an ip address.
the problem with zscaler is, that not all internal resources in a company are reachable through zscaler priave access.
so i wrote this dns server, which translates requests in the form of 10.1.2.3.ip.mycompany.com to 10.1.2.3.  That way, the zscaler dns is getting a prober IP and translates it into the 100.64. scope and devices formerly unreachable are accessible now.

this is work in progress.
