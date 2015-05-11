<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	exclude-result-prefixes="xs"
	version="2.0"
	xmlns:cmd="http://www.clarin.eu/cmd/"
	xmlns:xslx="http://www.w3.org/1999/XSL/Transform-dummy">
	
	<xsl:namespace-alias stylesheet-prefix="xslx" result-prefix="xsl"/>
	
	<xsl:param name="mapping-location" select="'./gsearch-mapping.xml'"/>
	<xsl:param name="mapping" select="doc($mapping-location)"/>
	
	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="xsl:stylesheet">
		<xsl:copy>
			<xsl:for-each select="$mapping//namespaces/namespace">
				<xsl:namespace name="{@ns}" select="@uri"/>
			</xsl:for-each>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="xsl:stylesheet/@version">
		<xsl:attribute name="version" select="'2.0'"/>
	</xsl:template>
	
	<xsl:template match="field[@name='foxml.all.text']">
		<xsl:comment>WARNING: Skipped the foxml.all.text as the needed extension isn't supported by Saxon HE!</xsl:comment>
	</xsl:template>

	<xsl:template match="xsl:for-each[exists(xsl:value-of[contains(@select,'exts:')])]">
		<xsl:comment>WARNING: Skipped the XSL command as the needed extension isn't supported by Saxon HE!</xsl:comment>
	</xsl:template>
	
	<xsl:template match="doc">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
			<xsl:call-template name="cmd"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template name="cmd">
		<xslx:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent[exists(cmd:CMD)]">
			<xsl:apply-templates select="$mapping" mode="cmd"/>
		</xslx:for-each>
	</xsl:template>
	
	<xsl:template match="text()" mode="cmd"/>
	
	<xsl:template match="field[exists(xpath) or exists(string[@expand!='true'])]" mode="cmd">
		<xsl:variable name="name" select="@name"/>
		<xslx:choose>
			<xsl:for-each select="xpath">
				<xslx:when test="normalize-space({.})!=''">
					<xslx:for-each select="{normalize-space(.)}">
						<field name="cmd.{$name}">
							<xslx:value-of select="normalize-space(.)"/>
						</field>
					</xslx:for-each>
				</xslx:when>
			</xsl:for-each>
			<xsl:if test="exists(string[@expand!='true'])">
				<xslx:otherwise>
					<field name="cmd.{$name}">
						<xsl:value-of select="string"/>
					</field>
				</xslx:otherwise>
			</xsl:if>
		</xslx:choose>
	</xsl:template>
	
</xsl:stylesheet>