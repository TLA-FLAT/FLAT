<?xml version="1.0"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="2.0"
  xmlns:cmd="http://www.clarin.eu/cmd/"
  xmlns:lat="http://lat.mpi.nl/"
  xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
  exclude-result-prefixes="cmd lat oai_dc"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:dcmitype="http://purl.org/dc/dcmitype/"
  xmlns:dcterms="http://purl.org/dc/terms/"
>
	<xsl:output method="xml" encoding="utf-8"/>
	
	<xsl:template match="/">
		<files>
			<file filepath="data/record.cmdi">
				<dcterms:identifier>
					<xsl:value-of select="/cmd:CMD/cmd:Header/cmd:MdSelfLink"/>
				</dcterms:identifier>
				<dcterms:format>application/x-cmdi+xml</dcterms:format>
			</file>
			<xsl:for-each select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy">
				<file filepath="data/{replace(cmd:ResourceRef/@lat:localURI,'.*/','')}">
					<dcterms:identifier>
						<xsl:value-of select="cmd:ResourceRef"/>
					</dcterms:identifier>
					<dcterms:format>
						<xsl:choose>
							<xsl:when test="normalize-space(cmd:ResourceType/@mimetype)!=''">
								<xsl:value-of select="cmd:ResourceType/@mimetype"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:text>application/octet-stream</xsl:text>
							</xsl:otherwise>
						</xsl:choose>
					</dcterms:format>
				</file>
			</xsl:for-each>
		</files>
	</xsl:template>
	
	<xsl:template match="@*">
		<xsl:copy/>
	</xsl:template>

	<xsl:template match="*">
		<xsl:element name="{name()}">
			<xsl:apply-templates select="@*|node()"/>
		</xsl:element>
	</xsl:template>
	
</xsl:stylesheet>
