<?xml version="1.0" encoding="UTF-8"?>
<!--
  Source : https://github.com/philipfennell/scissor-lift/blob/master/src/application/resources/xslt/marklogic/sl-trix-to-sem-triples.xsl
  License: https://github.com/philipfennell/scissor-lift/blob/master/LICENSE (Apache License Version 2.0, January 2004
  
  Changelog:
  - 20161019 MAW The root of a TriX document is TriX not trix
-->

<xsl:transform 
    xmlns:sem="http://marklogic.com/semantics"
    xmlns:trix="http://www.w3.org/2004/03/trix/trix-1/"
    xmlns:xd="http://www.oxygenxml.com/ns/doc/xsl"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    exclude-result-prefixes="trix xs xd"
    xpath-default-namespace="http://www.w3.org/2004/03/trix/trix-1/"
    version="2.0">
  
  <xd:doc scope="stylesheet">
    <xd:desc>
      <xd:p><xd:b>Created on:</xd:b> Feb 8, 2014</xd:p>
      <xd:p><xd:b>Author:</xd:b> Philip A. R. Fennell</xd:p>
      <xd:p>Converts TriX formated triples to MarkLogic sem:triples.</xd:p>
    </xd:desc>
  </xd:doc>
  
  <xsl:output encoding="UTF-8" indent="yes" media-type="application/xml" method="xml"/>
  
  
  <xd:doc>Root</xd:doc>
  <xsl:template match="/TriX/graph[triple]">
    <sem:triples>
      <xsl:apply-templates select="triple" mode="trix2sem"/>
    </sem:triples>
  </xsl:template>
  
  
  <xd:doc>Ignore the embedded graph URI</xd:doc>
  <xsl:template match="graph/uri" mode="trix2sem"/>
  
  
  <xd:doc>Transform a triple.</xd:doc>
  <xsl:template match="triple" mode="trix2sem">
    <sem:triple>
      <xsl:apply-templates select="*" mode="#current"/>
    </sem:triple>
  </xsl:template>
  
  
  <xd:doc>Set URI | IRI to corresponding subject, predicate or object according to position.</xd:doc>
  <xsl:template match="uri | iri" mode="trix2sem">
    <xsl:element name="{(if (position() eq 1) then 'sem:subject' else if (position() eq 2) then 'sem:predicate' else 'sem:object')}">
      <xsl:copy-of select="@*"/>
      <xsl:copy-of select="text()"/>
    </xsl:element>
  </xsl:template>
  
  
  <xd:doc>Set ID to corresponding subject or object according to position.</xd:doc>
  <xsl:template match="id" mode="trix2sem">
    <xsl:element name="{(if (position() eq 1) then 'sem:subject' else 'sem:object')}">
      <xsl:copy-of select="@*"/>
      <xsl:copy-of select="text()"/>
    </xsl:element>
  </xsl:template>
  
  
  <xd:doc>Typed literals plus their data type.</xd:doc>
  <xsl:template match="typedLiteral" mode="trix2sem">
    <sem:object>
      <xsl:copy-of select="@datatype"/>
      <xsl:copy-of select="text()"/>
    </sem:object>
  </xsl:template>
  
  
  <xd:doc>Plain literals plus their language code.</xd:doc>
  <xsl:template match="plainLiteral" mode="trix2sem">
    <sem:object>
      <xsl:copy-of select="@xml:lang"/>
      <xsl:copy-of select="text()"/>
    </sem:object>
  </xsl:template>
  
  
  <xd:doc>Ignore text nodes.</xd:doc>
  <xsl:template match="text()" mode="#all"/>
  
</xsl:transform>