<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	exclude-result-prefixes="xs"
	version="2.0"
	xmlns:cmd="http://www.clarin.eu/cmd/">
	
	<xsl:output method="xml" encoding="UTF-8"/>
	
	<xsl:param name="clarin_fc"/>
	<xsl:param name="profile_cache"/>
	
	<xsl:variable name="fc" select="document($clarin_fc)"/>
	
	<xsl:variable name="profiles" select="collection($profile_cache)"/>
	
	<xsl:template match="@*|node()" mode="#all">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:function name="cmd:findConceptPaths">
		<xsl:param name="concepts"/>
		<xsl:param name="multiple" as="xs:boolean"/>
		<xsl:variable name="paths" select="$profiles//*[@ConceptLink=$concepts]"/>
		<xsl:if test="exists($paths)">
			<xsl:choose>
				<xsl:when test="$multiple">
					<xsl:for-each-group select="$paths" group-by="ancestor::CMD_ComponentSpec//Header/ID">
						<xpath>
							<xsl:text>string-join(distinct-values(.//cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'</xsl:text>
							<xsl:value-of select="current-grouping-key()"/>
							<xsl:text>')])]/cmd:Components/(cmd:</xsl:text>
							<xsl:for-each select="current-group()">
								<xsl:value-of select="string-join(ancestor-or-self::*/@name,'/cmd:')"/>
								<xsl:text>/text()</xsl:text>
								<xsl:if test="position()!=last()">
									<xsl:text>,cmd:</xsl:text>
								</xsl:if>
							</xsl:for-each>
							<xsl:text>)),';')</xsl:text>
						</xpath>
					</xsl:for-each-group>
				</xsl:when>
				<xsl:otherwise>
					<xsl:for-each select="$paths">
						<xpath>
							<xsl:text>.//cmd:CMD[contains(cmd:Header/cmd:MdProfile,'</xsl:text>
							<xsl:value-of select="ancestor::CMD_ComponentSpec//Header/ID"/>
							<xsl:text>')]/cmd:Components/cmd:</xsl:text>
							<xsl:value-of select="string-join(ancestor-or-self::*/@name,'/cmd:')"/>
							<xsl:text>/text()</xsl:text>
						</xpath>
					</xsl:for-each>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:function>
	
	<xsl:template match="xpath/text()">
		<xsl:value-of select="normalize-space(.)"/>
	</xsl:template>
	
	<xsl:template match="cmd:facet">
		<xsl:apply-templates select="$fc//facetConcept[@name=current()]" mode="clarin"/>
	</xsl:template>
	
	<xsl:template match="cmd:concept">
		<xsl:copy-of select="cmd:findConceptPaths(current(),not(../@cmd:allowMultipleValues eq 'false'))"/>
	</xsl:template>
	
	<xsl:template match="facetConcept" mode="clarin">
		<xsl:copy-of select="cmd:findConceptPaths(concept,not(@allowMultipleValues eq 'false'))"/>
		<xsl:apply-templates select="pattern" mode="#current"/>
	</xsl:template>
	
	<xsl:template match="pattern" mode="clarin">
		<xpath>
			<xsl:value-of select="replace(replace(.,'c:','cmd:'),'(^|\.)(/)?/cmd:CMD','.//cmd:CMD')"/>
		</xpath>
	</xsl:template>
	
</xsl:stylesheet>