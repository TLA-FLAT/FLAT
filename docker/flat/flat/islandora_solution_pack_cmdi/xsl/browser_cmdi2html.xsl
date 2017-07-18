<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.1" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:tla="http://tla.mpi.nl/" xmlns="http://www.w3.org/1999/xhtml">
    
    <!--
    Based on "Stylesheet for visualisation of CMDI records" by Twan Goosen
    -->

    <xsl:output encoding="UTF-8" indent="yes" method="xml" omit-xml-declaration="yes" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

    <xsl:template match="/">
            <div>
                <h2>Component Metadata</h2>
                <ul class="panel-group" id="a{generate-id(//cmd:Components)}">
                    <xsl:apply-templates select="//cmd:Components/*" mode="components"/>
                </ul>
            </div>
    </xsl:template>
    
    <xsl:template match="*" mode="components">
        <li class="panel panel-default">
            <xsl:choose>
                <xsl:when test="count(./*) &gt; 0">
                    <div class="panel-heading">
                        <div class="panel-title">
                            <a href="#c{generate-id(.)}" data-toggle="collapse" aria-expanded="false">
                            <xsl:choose>
                                    <xsl:when test="parent::cmd:Components">
                                        <xsl:attribute name="class"></xsl:attribute>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <xsl:attribute name="class">collapsed</xsl:attribute>
                                    </xsl:otherwise>
                                </xsl:choose>
                                <strong>
                                    <xsl:value-of select="local-name(.)"/>
                                </strong>
                            </a>
                        </div>
                    </div>
                    <div id="c{generate-id(.)}" aria-expanded="false">
                        <xsl:choose>
                            <xsl:when test="parent::cmd:Components">
                                <xsl:attribute name="class">panel-collapse collapse in</xsl:attribute>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="class">panel-collapse collapse</xsl:attribute>
                            </xsl:otherwise>
                        </xsl:choose>
                        <div class="panel-body">
                            <ul class="panel-group" id="a{generate-id(.)}">
                                <xsl:apply-templates select="./*" mode="components"/>
                            </ul>
                        </div>
                    </div>
                </xsl:when>
                <xsl:otherwise>
                    <!-- ordinary label -->
                    <strong>
                        <xsl:value-of select="local-name(.)"/>
                    </strong>
                    <xsl:text> : </xsl:text>
                    <xsl:if test="normalize-space(text()) != ''">
                        <span>
                            <xsl:value-of select="text()"/>
                        </span>
                    </xsl:if>
                </xsl:otherwise>
            </xsl:choose>
        </li>
    </xsl:template>

</xsl:stylesheet>
