<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    exclude-result-prefixes="xs"
    version="2.0"
    xmlns:xslx="alias">
    
    <xsl:namespace-alias stylesheet-prefix="xslx" result-prefix="xsl"/>

    <xsl:param name="cmd2dc" select="()"/>
    <xsl:param name="cmd2other" select="()"/>

    <xsl:template match="node() | @*">
        <xsl:copy>
            <xsl:apply-templates select="node() | @*"/>
        </xsl:copy>
    </xsl:template>
    
    <!-- add the import to the top -->
    <xsl:template match="xsl:stylesheet">
        <xsl:copy>
            <xsl:apply-templates select="@*"/>
            <xsl:if test="normalize-space($cmd2dc)!=''">
                <xslx:import href="{$cmd2dc}"/>
            </xsl:if>
            <xsl:if test="normalize-space($cmd2other)!=''">
                <xslx:import href="{$cmd2other}"/>
            </xsl:if>
            <xsl:apply-templates select="node()"/>
        </xsl:copy>
    </xsl:template>
    
    <!-- remove the dummy DC template -->
    <xsl:template match="xsl:template[@mode='dc'][normalize-space($cmd2dc)!='']"/>
    
    <!-- remove the dummy other template -->
    <xsl:template match="xsl:template[@mode='other'][normalize-space($cmd2other)!='']"/>

</xsl:stylesheet>