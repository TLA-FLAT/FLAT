<?xml version="1.0"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="2.0"
  xmlns:cmd="http://www.clarin.eu/cmd/"
  xmlns:lat="http://lat.mpi.nl/"
>
	<xsl:output method="xml" encoding="utf-8"/>
	
	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="cmd:ResourceRef/@lat:flatURI"/>
	
	<xsl:template match="cmd:ResourceRef/@lat:localURI">
		<xsl:attribute name="lat:localURI" select="replace(.,'.*/','\${easy-dataset}/original/')"/>
	</xsl:template>
	
</xsl:stylesheet>
