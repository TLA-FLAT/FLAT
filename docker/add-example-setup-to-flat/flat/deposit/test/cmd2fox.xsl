<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:foxml="info:fedora/fedora-system:def/foxml#"
    xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:lat="http://lat.mpi.nl/" version="3.0">

	<xsl:import href="jar:cmd2fox.xsl"/>
    
    <xsl:template match="cmd:CMD" mode="dc">
		<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
			xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/"
			xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
			xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
			<dc:name>
				<xsl:value-of select="(//cmd:ComicBook/cmd:Title)|(//cmd:CoreCollectionInformation/cmd:title)"/>
			</dc:name>
			<dc:title>
				<xsl:value-of select="(//cmd:ComicBook/cmd:Title)|(//cmd:CoreCollectionInformation/cmd:title)"/>
			</dc:title>
		</oai_dc:dc>
	</xsl:template>

	<xsl:template match="text()" mode="dc"/>
	
	<xsl:template match="cmd:ResourceProxy" mode="thumbnail">
		<xsl:param name="resURI" tunnel="yes"/>
		<xsl:param name="resMIME" tunnel="yes"/>
		<xsl:choose>
			<xsl:when test="starts-with($resMIME,'application/pdf')">
				<foxml:contentLocation TYPE="URL" REF="file:{$icon-base}/comic.png"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:next-match/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	
	<!-- checksum -->
	<xsl:template match="cmd:ResourceProxy" mode="checksum">
		<xsl:param name="base" select="base-uri()" tunnel="yes"/>
		<xsl:param name="resURI" tunnel="yes"/>
		<xsl:variable name="res" select="current()"/>
		<xsl:choose xmlns:dc="http://purl.org/dc/elements/1.1/">
			<!--
			NOTE: implement here your own way to get the md5 checksum
			<xsl:when test="">
				<xsl:variable name="md5" select="doc($checksum)/md5"/>
				<xsl:choose>
					<xsl:when test="normalize-space($md5)!=''">
						<dc:identifier>
							<xsl:text>md5:</xsl:text>
							<xsl:value-of select="$md5"/>
						</dc:identifier>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message>ERR: no checksum for resource[<xsl:value-of select="$resURI"/>]</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			-->
			<xsl:when test="false()"/>
			<xsl:otherwise>
				<xsl:message>DBG: use default checksum for resource[<xsl:value-of select="$resURI"/>]</xsl:message>
				<xsl:next-match/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>	

</xsl:stylesheet>