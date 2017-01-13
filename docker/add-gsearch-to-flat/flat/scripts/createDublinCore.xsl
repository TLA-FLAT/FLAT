<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs" version="2.0" xmlns:lat="http://lat.mpi.nl/" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:xslx="http://www.w3.org/1999/XSL/Transform-dummy">

	<xsl:namespace-alias stylesheet-prefix="xslx" result-prefix="xsl"/>

	<xsl:param name="mapping-location" select="'./gsearch-mapping.xml'"/>
	<xsl:param name="mapping" select="doc($mapping-location)"/>

	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@*"/>
			<xsl:apply-templates select="lat:field"/>
			<xsl:apply-templates select="node() except lat:field"/>
		</xsl:copy>
	</xsl:template>


	<xsl:template match="lat:field">
		<xsl:variable name="name" select="@name"/>
		<xsl:variable name="map"  select="$mapping//field[@name=$name]"/>
		<xsl:attribute name="{@attribute}">
			<xsl:value-of select="concat('(',string-join($map/xpath,','),')')"/>
		</xsl:attribute>
	</xsl:template>

</xsl:stylesheet>
