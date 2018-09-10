<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" exclude-result-prefixes="xs" version="2.0" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:xslx="http://www.w3.org/1999/XSL/Transform-dummy">

	<xsl:namespace-alias stylesheet-prefix="xslx" result-prefix="xsl"/>

	<xsl:param name="mapping-location" select="'./gsearch-mapping.xml'"/>
	<xsl:param name="mapping" select="doc($mapping-location)"/>

	<xsl:template match="@* | node()">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="xsl:stylesheet">
		<xsl:copy>
			<xsl:for-each-group select="$mapping//namespaces/namespace" group-by="@ns">
				<xsl:namespace name="{current-grouping-key()}" select="current-group()[1]/@uri"/>
			</xsl:for-each-group>
			<xsl:apply-templates select="@*"/>
			<xsl:call-template name="cmd-keys"/>
			<xsl:apply-templates select="node()"/>
		</xsl:copy>
	</xsl:template>

	<xsl:template match="doc">
		<xsl:copy>
			<xsl:apply-templates select="@* | node()"/>
			<xsl:call-template name="cmd"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template name="cmd-keys">
		<xsl:apply-templates select="$mapping" mode="cmd-keys"/>
	</xsl:template>

	<xsl:template match="text()" mode="cmd-keys"/>

	<xsl:template match="field[exists(xpath)]" mode="cmd-keys">
		<xsl:variable name="name" select="@name"/>
		<xsl:for-each select="xpath">
			<xsl:variable name="xp" select="."/>
			<xsl:if test="matches($xp, '^/')">
				<xslx:key name="cmd-key-{$name}-{position()}" match="{replace($xp,'/cmd:CMD','cmd:CMD')}" use="string()"/>
			</xsl:if>
		</xsl:for-each>
	</xsl:template>

	<xsl:template name="cmd">
		<xslx:for-each select="foxml:datastream/foxml:datastreamVersion[last()]/foxml:xmlContent[cmd:CMD]">
			<xsl:apply-templates select="$mapping" mode="cmd"/>
		</xslx:for-each>
	</xsl:template>

	<xsl:template match="text()" mode="cmd"/>

	<xsl:template match="field[exists(xpath) or exists(string[@expand != 'true'])]" mode="cmd">
		<xsl:variable name="name" select="@name"/>
		<xslx:choose>
			<xsl:for-each select="xpath">
				<xsl:variable name="xp" select="."/>
				<xsl:choose>
					<xsl:when test="matches($xp, '^/')">
						<xslx:when test="{normalize-space(replace($xp,'/cmd:CMD','cmd:CMD'))}[normalize-space()!='']">
							<xslx:for-each select="{normalize-space(replace($xp,'/cmd:CMD','cmd:CMD'))}[normalize-space()!='']">
								<!-- process only the last instance, as FOXML stores them in order, so the last is within the current (last) version -->
								<xslx:if test="generate-id(current()) = generate-id(key('cmd-key-{$name}-{position()}', string())[last()])">
									<field name="cmd.{$name}">
										<xsl:choose>
											<xsl:when test="normalize-space(@val) != ''">
												<xslx:value-of select="{@val}"/>
											</xsl:when>
											<xsl:otherwise>
												<xslx:value-of select="normalize-space(.)"/>
											</xsl:otherwise>
										</xsl:choose>
									</field>
								</xslx:if>
							</xslx:for-each>
						</xslx:when>
					</xsl:when>
					<xsl:otherwise>
						<xslx:when test="{normalize-space($xp)}">
							<field name="cmd.{$name}">
								<xsl:choose>
									<xsl:when test="normalize-space(@val) != ''">
										<xslx:value-of select="{@val}"/>
									</xsl:when>
									<xsl:otherwise>
										<xslx:value-of select="normalize-space(.)"/>
									</xsl:otherwise>
								</xsl:choose>
							</field>
						</xslx:when>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
			<xsl:if test="exists(string[@expand != 'true'])">
				<xslx:otherwise>
					<field name="cmd.{$name}">
						<xsl:value-of select="string"/>
					</field>
				</xslx:otherwise>
			</xsl:if>
		</xslx:choose>
	</xsl:template>

</xsl:stylesheet>
