<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:cmd="http://www.clarin.eu/cmd/"
                version="1.0">

    <xsl:template match="node() | @*">
        <xsl:copy>
            <xsl:apply-templates select="node() | @*"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="cmd:*">
        <xsl:element name="cmd:{local-name()}">
            <xsl:apply-templates select="node() | @*"/>
        </xsl:element>
    </xsl:template>

</xsl:stylesheet>