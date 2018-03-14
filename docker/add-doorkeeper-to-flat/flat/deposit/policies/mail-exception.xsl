<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="2.0">

    <xsl:param name="bag"/>
    <xsl:param name="sip"/>
    <xsl:param name="user"/>
    <xsl:param name="exception"/>
    <xsl:param name="stacktrace"/>
	<xsl:param name="repo"/>

    <xsl:output method="html" encoding="UTF-8"/>

    <xsl:template match="/">
        <p>
            <b> <xsl:text> Dear Admin, a deposit failed! Please have a look at: </xsl:text> </b>
            <xsl:value-of select="$bag"/>
            <br/>
            <b><xsl:text> for SIP[ </xsl:text></b>
            <xsl:value-of select="$sip"/>
            <b><xsl:text>]</xsl:text></b>
            <br/>
            <xsl:if test="normalize-space($user)!=''">
                <b><xsl:text> for user[ </xsl:text></b>
                <xsl:value-of select="$user"/>
                <b><xsl:text>]</xsl:text></b>
                <br/>
             </xsl:if>
        </p>
        <p>
            <xsl:if test="normalize-space($exception)!=''">
                <b><xsl:text>Exception: </xsl:text></b>
                <xsl:value-of select="$exception"/>
                 <br/>
				 <br/>
                <b><xsl:text>Detailed:</xsl:text></b> 
                <xsl:value-of select="$stacktrace"/>
            </xsl:if>
        </p>
    </xsl:template>

</xsl:stylesheet>