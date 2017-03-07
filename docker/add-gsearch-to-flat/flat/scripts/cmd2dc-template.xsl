<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="3.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:foxml="info:fedora/fedora-system:def/foxml#"
    xmlns:cmd="http://www.clarin.eu/cmd/"
    xmlns:cmdp="http://www.clarin.eu/cmd/"
    xmlns:lat="http://lat.mpi.nl/"
    xmlns:iso="http://www.iso.org/"
    xmlns:sil="http://www.sil.org/">
	
	<xsl:import href="jar:cmd2fox.xsl"/>
	
	<!-- DUBLIN CORE -->
    
    <xsl:template match="cmd:CMD" mode="dc">
		<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
			xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/"
			xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
			xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
			<xsl:variable name="title">
				<lat:field name="Name" attribute="select"/>
			</xsl:variable>
			<xsl:if test="exists($title[normalize-space() != ''][1])">
				<dc:title>
					<xsl:value-of select="$title[normalize-space() != ''][1]"/>
				</dc:title>
			</xsl:if>
			<xsl:variable name="descr">
				<lat:field name="Description" attribute="select"/>					
			</xsl:variable>
			<xsl:if test="exists($descr[normalize-space() != ''][1])">
				<dc:description>
					<xsl:value-of select="$descr[normalize-space() != ''][1]"/>
				</dc:description>
			</xsl:if>
		</oai_dc:dc>
	</xsl:template>

</xsl:stylesheet>