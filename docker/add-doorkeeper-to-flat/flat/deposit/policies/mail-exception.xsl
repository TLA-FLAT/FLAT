<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:lat="http://lat.mpi.nl/" version="2.0">
    <xsl:param name="bag"/>
    <xsl:param name="sip"/>
    <xsl:param name="user"/>
    <xsl:param name="exception"/>
    <xsl:param name="stacktrace"/>
    <xsl:param name="repo"/>
    <xsl:param name="outcome"/>
    <xsl:output method="html" encoding="UTF-8"/>
    <xsl:template match="/">
        <p>
            <b> <xsl:text> The result of the deposit: </xsl:text> </b>
            <xsl:value-of select="$outcome"/>
            <br/>
            <xsl:if test="normalize-space($user)!=''">
                <b><xsl:text> User: </xsl:text></b>
                <xsl:value-of select="$user"/>
                <br/>
            </xsl:if>
            <b><xsl:text> SIP: </xsl:text></b>
            <xsl:value-of select="$sip"/>
            <br/>
            <xsl:variable name="localURI" select="(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy/cmd:ResourceRef/@lat:localURI)[1]"/>
            <xsl:if test="normalize-space($localURI)!=''">
                <b> <xsl:text> Data Location: </xsl:text> </b>
                <xsl:value-of select="replace(resolve-uri($localURI,base-uri()),'(.*)/.*','$1')"/>
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
