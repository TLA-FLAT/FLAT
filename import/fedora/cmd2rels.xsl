<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:cmd="http://www.clarin.eu/cmd/"
	exclude-result-prefixes="xs"
	version="2.0">
	
	<xsl:param name="dir" select="'./'"/>
	
	<xsl:template name="main">
		<relations>
			<xsl:for-each select="collection(concat($dir,'?select=*.cmdi;recurse=yes'))">
				<xsl:variable name="rec" select="current()"/>
				<xsl:variable name="src" select="base-uri($rec)"/>
				<xsl:variable name="frm" select="$rec/cmd:CMD/cmd:Header/cmd:MdSelfLink"/>
				<xsl:for-each select="$rec/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy">
					<relation>
						<src>
							<xsl:value-of select="$src"/>
						</src>
						<from>
							<xsl:value-of select="$frm"/>
						</from>
						<to>
							<xsl:choose>
								<xsl:when test="starts-with(cmd:ResourceRef,'hdl:')">
									<xsl:value-of select="cmd:ResourceRef"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:value-of select="resolve-uri(cmd:ResourceRef,$src)"/>
								</xsl:otherwise>
							</xsl:choose>
						</to>
						<type>
							<xsl:value-of select="cmd:ResourceType"/>
						</type>
					</relation>
				</xsl:for-each>
			</xsl:for-each>
		</relations>
	</xsl:template>
	
	<xsl:template match="/">
		<xsl:call-template name="main"/>
	</xsl:template>
</xsl:stylesheet>