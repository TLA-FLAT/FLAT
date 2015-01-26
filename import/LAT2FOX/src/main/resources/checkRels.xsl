<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:sx="java:nl.mpi.tla.saxon" 
	exclude-result-prefixes="xs"
	version="2.0">
	
	<xsl:param name="rels-uri" select="'./relations.xml'"/>
	<xsl:param name="rels-doc" select="document($rels-uri)"/>
	<xsl:key name="rels-from" match="relation" use="from"/>
	<xsl:key name="rels-to" match="relation" use="to"/>
	
	<xsl:template name="main">
		<xsl:for-each-group select="$rels-doc//relation" group-by="from">
			<xsl:variable name="recs" select="distinct-values(current-group()/src)"/>
			<xsl:if test="count($recs)!=1">
				<xsl:message>WRN: handle[<xsl:value-of select="current-grouping-key()"/>] is used for multiple records[<xsl:value-of select="string-join($recs,', ')"/>]!</xsl:message>
			</xsl:if>
			<xsl:if test="not(sx:checkURL(replace(current-grouping-key(),'^hdl:','http://hdl.handle.net/')))">
				<xsl:message>WRN: malformed URL[<xsl:value-of select="current-grouping-key()"/>] appears in [<xsl:value-of select="string-join(distinct-values(current-group()/src),', ')"/>]!</xsl:message>
			</xsl:if>
			<xsl:if test="exists($rels-doc/key('rels-to',current-grouping-key())[type='Resource'])">
				<xsl:message>WRN: metadata PID[<xsl:value-of select="current-grouping-key()"/>] is also used as Resource PID to [<xsl:value-of select="string-join($rels-doc/key('rels-to',current-grouping-key())[type='Resource']/dst,', ')"/>] by [<xsl:value-of select="string-join($rels-doc/key('rels-to',current-grouping-key())[type='Resource']/src,', ')"/>]!</xsl:message>
			</xsl:if>
		</xsl:for-each-group>
		<xsl:for-each-group select="$rels-doc//relation" group-by="to">
			<xsl:variable name="res" select="distinct-values(current-group()[type='Resource'][normalize-space(dst)!='']/resolve-uri(dst,src))"/>
			<xsl:if test="count($res) gt 1">
				<xsl:message>WRN: handle[<xsl:value-of select="current-grouping-key()"/>] is used for multiple resources[<xsl:value-of select="count($res)"/>][<xsl:value-of select="string-join($res,', ')"/>]!</xsl:message>
			</xsl:if>
			<xsl:if test="not(sx:checkURL(replace(current-grouping-key(),'^hdl:','http://hdl.handle.net/')))">
				<xsl:message>WRN: malformed URL[<xsl:value-of select="current-grouping-key()"/>] appears in [<xsl:value-of select="string-join(distinct-values(current-group()/src),', ')"/>]!</xsl:message>
			</xsl:if>
		</xsl:for-each-group>
	</xsl:template>
	
	<xsl:template match="/">
		<xsl:call-template name="main"/>
	</xsl:template>
	
</xsl:stylesheet>