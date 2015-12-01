FROM		flat
MAINTAINER	Menzo Windhouwer <menzo.windhouwer@meertens.knaw.nl>

RUN apt-get update &&\
	apt-get -y dist-upgrade &&\
	apt-get -y install ant

#
# Add FLAT's own scripts and tools 
#

RUN mkdir -p /app/flat
ADD flat/scripts/* /app/flat/
RUN chmod +x /app/flat/*.sh

#Clean up APT when done.
RUN apt-get clean &&\
	rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
