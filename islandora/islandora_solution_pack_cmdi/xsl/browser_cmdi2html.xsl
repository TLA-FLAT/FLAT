<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.1" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:tla="http://tla.mpi.nl/" xmlns="http://www.w3.org/1999/xhtml">
    
    <!--
        Stylesheet for visualisation of CMDI records. 
        Aimed to be executed client-side in the browser (therefore should be strictly XSLT 1.1)
        
        Author: Twan Goosen (twan.goosen@mpi.nl)
        Date: February 2015
	Version: 1.0
    -->

    <xsl:output encoding="UTF-8" indent="yes" method="xml" omit-xml-declaration="yes" doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"/>

    <xsl:param name="headerLogoUrl">https://corpus1.mpi.nl/cmdi-xslt-1.0/tla.png</xsl:param>

    <xsl:variable name="title">
        <xsl:call-template name="title"/>
    </xsl:variable>

    <xsl:template match="/">
            <div>
                <h2>Component Metadata</h2>
                <ul>
                    <xsl:apply-templates select="//cmd:Components/*" mode="components"/>
                </ul>
            </div>
    </xsl:template>
    
    <xsl:template mode="LandingPage" match="cmd:ResourceProxy[cmd:ResourceType='LandingPage']">
        <a id="viewInContext">
            <xsl:attribute name="href">
                <xsl:apply-templates select="cmd:ResourceRef" mode="resolveHandle"/>
            </xsl:attribute>
            <xsl:text>View in archive context</xsl:text>
        </a>
    </xsl:template>
    
    <xsl:template mode="LandingPage" match="cmd:MdSelfLink">
        <xsl:variable name="selfLink">
            <xsl:apply-templates select="." mode="resolveHandle"/>
        </xsl:variable>
        <xsl:if test="starts-with($selfLink, 'http://hdl.handle.net/1839') or starts-with($selfLink, 'http://hdl.handle.net/11142')">
            <a id="viewInContext">
                <xsl:attribute name="href">
                    <xsl:value-of select="concat($selfLink,'@view')"/>
                </xsl:attribute>
                <xsl:text>View in archive context</xsl:text>
            </a>
        </xsl:if>
    </xsl:template>

    <xsl:template match="cmd:Header">
        <ul>
            <li> This record: <xsl:apply-templates select="cmd:MdSelfLink"/>
            </li>
            <li>Creation date: <xsl:value-of select="cmd:MdCreationDate"/></li>
            <li>Creator: <xsl:value-of select="cmd:MdCreator"/></li>
        </ul>
    </xsl:template>

    <xsl:template match="cmd:MdSelfLink">
        <!-- a link to the current record with the title as text if available -->
        <a>
            <xsl:attribute name="href">
                <xsl:apply-templates select="." mode="resolveHandle"/>
            </xsl:attribute>
            <xsl:choose>
                <xsl:when test="normalize-space($title) != ''">
                    <xsl:value-of select="$title"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="text()"/>
                </xsl:otherwise>
            </xsl:choose>
        </a>
    </xsl:template>

    <xsl:template match="cmd:ResourceProxy">
        <li>
            <!-- TODO: Icon (or css class) depending on mime type -->
            <xsl:value-of select="cmd:ResourceType"/>: 
            <a>
                <xsl:attribute name="id" select="@id"/>
                <xsl:attribute name="href">
                    <xsl:apply-templates select="cmd:ResourceRef" mode="resolveHandle"/>
                </xsl:attribute>
                <xsl:apply-templates select="cmd:ResourceRef" mode="displayHandle"/>
            </a>
        </li>
    </xsl:template>

    <xsl:template name="title">
        <xsl:variable name="sessiontitle" select="//cmd:lat-session/cmd:Name"/>
        <xsl:variable name="corpustitle" select="//cmd:lat-corpus/cmd:Name"/>
        <xsl:variable name="collectiontitle" select="//cmd:CollectionInfo/cmd:Name"/>
        <xsl:choose>
            <xsl:when test="normalize-space($sessiontitle) != ''">
                <xsl:value-of select="$sessiontitle"/>
            </xsl:when>
            <xsl:when test="normalize-space($corpustitle) != ''">
                <xsl:value-of select="$corpustitle"/>
            </xsl:when>
            <xsl:when test="normalize-space($collectiontitle) != ''">
                <xsl:value-of select="$collectiontitle"/>
            </xsl:when>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="*" mode="components">
        <li>
            <xsl:choose>
                <xsl:when test="@ref">
                    <!-- make this label a link to referenced resource -->
                    <a>
                        <xsl:attribute name="href">
                            <xsl:apply-templates select="//cmd:ResourceProxy[@id=current()/@ref]/cmd:ResourceRef" mode="resolveHandle"/>
                        </xsl:attribute>
                        <strong>
                            <xsl:value-of select="name(.)"/>
                        </strong>
                    </a>
                </xsl:when>
                <xsl:otherwise>
                    <!-- ordinary label -->
                    <strong>
                        <xsl:value-of select="name(.)"/>
                    </strong>
                </xsl:otherwise>
            </xsl:choose>
            <xsl:if test="normalize-space(text()) != ''">
                <span>
                    <xsl:value-of select="text()"/>
                </span>
            </xsl:if>
            <xsl:if test="count(./*) &gt; 0">
                <ul>
                    <xsl:apply-templates select="./*" mode="components"/>
                </ul>
            </xsl:if>
        </li>
    </xsl:template>
    
    <xsl:template match="*" mode="resolveHandle">
        <xsl:choose>
            <xsl:when test="starts-with(.,'hdl:')">
                http://hdl.handle.net/<xsl:value-of select="substring-after(., 'hdl:')"/>                            
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="text()"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    <xsl:template match="*" mode="displayHandle">
        <xsl:choose>
            <xsl:when test="starts-with(.,'hdl:')">
                <xsl:value-of select="substring-after(., 'hdl:')"/>                            
            </xsl:when>
            <xsl:when test="starts-with(.,'http://hdl.handle.net/')">
                <xsl:value-of select="substring-after(., 'http://hdl.handle.net/')"/>                            
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="."/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
</xsl:stylesheet>
