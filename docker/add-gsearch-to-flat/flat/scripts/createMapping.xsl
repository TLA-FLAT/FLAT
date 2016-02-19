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
		<xsl:param name="val"/>
		<xsl:variable name="paths" select="$profiles//*[@ConceptLink=$concepts]"/>
		<xsl:if test="exists($paths)">
			<xsl:choose>
				<xsl:when test="$multiple">
					<xsl:for-each-group select="$paths" group-by="ancestor::CMD_ComponentSpec//Header/ID">
						<xpath>
							<xsl:if test="exists($val)">
								<xsl:attribute name="val" select="$val"/>
							</xsl:if>
							<xsl:for-each select="current-group()">
								<xsl:text>/cmd:CMD[cmd:Header/cmd:MdProfile[contains(.,'</xsl:text>
								<xsl:value-of select="current-grouping-key()"/>
								<xsl:text>')]]/cmd:Components/cmd:</xsl:text>
								<xsl:value-of select="string-join(ancestor-or-self::*/@name,'/cmd:')"/>
								<xsl:if test="position()!=last()">
									<xsl:text>|</xsl:text>
								</xsl:if>
							</xsl:for-each>
						</xpath>
					</xsl:for-each-group>
				</xsl:when>
				<xsl:otherwise>
					<xsl:for-each select="$paths">
						<xpath>
							<xsl:if test="exists($val)">
								<xsl:attribute name="val" select="$val"/>
							</xsl:if>
							<xsl:text>/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'</xsl:text>
							<xsl:value-of select="ancestor::CMD_ComponentSpec//Header/ID"/>
							<xsl:text>')]/cmd:Components/cmd:</xsl:text>
							<xsl:value-of select="string-join(ancestor-or-self::*/@name,'/cmd:')"/>
						</xpath>
					</xsl:for-each>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:function>
	
	<xsl:function name="cmd:findConceptPaths">
		<xsl:param name="concepts"/>
		<xsl:param name="multiple" as="xs:boolean"/>
		<xsl:sequence select="cmd:findConceptPaths($concepts,$multiple,())"></xsl:sequence>
	</xsl:function>
	
	<xsl:template match="namespaces">
		<xsl:copy>
			<xsl:apply-templates select="@*"/>
			<namespace ns="cmd" uri="http://www.clarin.eu/cmd/"/>
			<namespace ns="set" uri="http://exslt.org/sets"/>
			<xsl:apply-templates/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="xpath/text()">
		<xsl:value-of select="normalize-space(.)"/>
	</xsl:template>
	
	<xsl:template match="cmd:facet">
		<xsl:apply-templates select="$fc//facetConcept[@name=current()]" mode="clarin"/>
	</xsl:template>
	
	<xsl:template match="cmd:concept">
		<xsl:copy-of select="cmd:findConceptPaths(current(),not(../@multiValued eq 'false'),@val)"/>
	</xsl:template>
	
	<xsl:template match="facetConcept" mode="clarin">
		<xsl:copy-of select="cmd:findConceptPaths(concept,not(@allowMultipleValues eq 'false'),@val)"/>
		<xsl:apply-templates select="pattern" mode="#current"/>
	</xsl:template>
	
	<xsl:template match="pattern" mode="clarin">
		<xpath>
			<xsl:value-of select="replace(replace(replace(replace(.,'c:','cmd:'),'(^|\.)(/)?/cmd:CMD','/cmd:CMD'),'//text\(\)','//*'),'/text\(\)','')"/>
		</xpath>
	</xsl:template>
	
</xsl:stylesheet>