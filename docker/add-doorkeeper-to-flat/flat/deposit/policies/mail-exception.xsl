<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="2.0">
    
    <xsl:param name="bag"/>
    <xsl:param name="sip"/>
    <xsl:param name="user"/>
    <xsl:param name="exception"/>
    
    <xsl:output method="html" encoding="UTF-8"/>
    
    <xsl:template match="/">
        <p>
            <xsl:text>Dear FLAT admin, a deposit failed! Please have a look at: </xsl:text>
            <xsl:value-of select="$bag"/>
            <xsl:text> for SIP[</xsl:text>
            <xsl:value-of select="$sip"/>
            <xsl:text>]</xsl:text>
            <xsl:if test="normalize-space($user)!=''">
                <xsl:text> for user[</xsl:text>
                <xsl:value-of select="$user"/>
                <xsl:text>]</xsl:text>
             </xsl:if> 
        </p>
        <p>
            <xsl:if test="normalize-space($exception)!=''">
                <xsl:text>Exception: </xsl:text>
                <xsl:value-of select="$exception"/>
            </xsl:if> 
        </p>
    </xsl:template>
    
</xsl:stylesheet>