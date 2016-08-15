<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    exclude-result-prefixes="xs"
    version="2.0">
    
    <xsl:param name="status" select="()"/>
    <xsl:param name="pid" select="()"/>
    <xsl:param name="fid" select="()"/>
    
    <xsl:output method="xml" encoding="utf-8"/>
    
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>
    
    <xsl:template match="/">
        <result>
            <xsl:attribute name="status">
                <xsl:choose>
                    <xsl:when test="exists($status) and $status=true()">
                        <xsl:sequence select="'succeeded'"/>
                    </xsl:when>
                    <xsl:when test="exists($status) and $status=false()">
                        <xsl:sequence select="'failed'"/>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:sequence select="'executing'"/>
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:if test="exists($pid)">
                <xsl:attribute name="pid" select="$pid"/>
            </xsl:if>
            <xsl:if test="exists($fid)">
                <xsl:attribute name="fid" select="$fid"/>
            </xsl:if>
            <xsl:apply-templates/>
        </result>
    </xsl:template>
    
</xsl:stylesheet>