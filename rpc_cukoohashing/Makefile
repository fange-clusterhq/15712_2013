################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################

default: server client

server:
	@gcc lookup3.c cuckoo_hash.c handle_rpc.c server.c -o server -Wall -Werror -g -std=c99

client:
	@gcc client.c -o client -g -Wall -Werror

clean:
	@rm server client
