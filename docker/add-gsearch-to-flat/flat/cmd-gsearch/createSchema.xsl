<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	exclude-result-prefixes="xs cmd"
	version="2.0"
	xmlns:cmd="http://www.clarin.eu/cmd/">
	
	<xsl:param name="mapping-location" select="'./gsearch-mapping.xml'"/>
	<xsl:param name="mapping" select="doc($mapping-location)"/>
	
	<xsl:output name="xml" indent="yes"/>
	
	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="field[@name='PID']">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
		<xsl:for-each select="$mapping/mapping-table/mappings/field">
			<field name="cmd.{@name}" indexed="true" stored="true">
				<xsl:attribute name="type" select="if (exists(@type)) then (@type) else ('text_general')"/>
				<xsl:attribute name="multiValued" select="not(../@multiValued eq 'false')"/>
			</field>
		</xsl:for-each>
	</xsl:template>
		
</xsl:stylesheet>