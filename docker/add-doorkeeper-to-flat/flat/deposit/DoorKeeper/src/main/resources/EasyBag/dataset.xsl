<?xml version="1.0"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  version="2.0"
  xmlns:cmd="http://www.clarin.eu/cmd/"
  xmlns:lat="http://lat.mpi.nl/"
  xmlns:foxml="info:fedora/fedora-system:def/foxml#"
  xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
  exclude-result-prefixes="cmd lat foxml oai_dc"
  xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:dcmitype="http://purl.org/dc/dcmitype/"
  xmlns:dcterms="http://purl.org/dc/terms/"
>
	<xsl:output method="xml" encoding="utf-8"/>
	
	<xsl:param name="creator" select="()"/>
	<xsl:param name="audience" select="()"/>
	<xsl:param name="accessRights" select="'NO ACCESS'"/>
	
	<xsl:template match="/">
		<xsl:variable name="DC" select="(//foxml:datastream[@ID='DC']/foxml:datastreamVersion)[last()]//oai_dc:dc"/>
		<ddm:DDM
			xmlns:dc="http://purl.org/dc/elements/1.1/"
			xmlns:dcmitype="http://purl.org/dc/dcmitype/"
			xmlns:narcis="http://easy.dans.knaw.nl/schemas/vocab/narcis-type/"
			xmlns:dcx="http://easy.dans.knaw.nl/schemas/dcx/"
			xmlns:dcx-dai="http://easy.dans.knaw.nl/schemas/dcx/dai/"
			xmlns:ddm="http://easy.dans.knaw.nl/schemas/md/ddm/"
			xmlns:dcterms="http://purl.org/dc/terms/"
			xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
			xmlns:id-type="http://easy.dans.knaw.nl/schemas/vocab/identifier-type/">
			<ddm:profile>
				<dc:title>
					<xsl:value-of select="$DC/dc:title"/>
				</dc:title>
				<dcterms:description>
					<xsl:value-of select="$DC/dc:description"/>
				</dcterms:description>
				<dcx-dai:creatorDetails>
					<xsl:choose>
						<xsl:when test="exists($creator)">
							<xsl:copy-of select="document(concat('file:',$creator))"/>
						</xsl:when>
						<xsl:otherwise>
							<dcx-dai:author>
								<dcx-dai:titles></dcx-dai:titles>
								<dcx-dai:initials>I</dcx-dai:initials>
								<dcx-dai:insertions></dcx-dai:insertions>
								<dcx-dai:surname>DoorKeeper</dcx-dai:surname>
								<dcx-dai:organization>
									<dcx-dai:name xml:lang="en">FLAT</dcx-dai:name>
								</dcx-dai:organization>
							</dcx-dai:author>
						</xsl:otherwise>
					</xsl:choose>
				</dcx-dai:creatorDetails>        
				<ddm:created>
					<xsl:value-of select="format-date(current-date(), '[Y0001]-[M01]-[D01]')"/>
				</ddm:created>
				<ddm:audience>
					<xsl:value-of select="$audience"/>
				</ddm:audience>
				<ddm:accessRights>
					<xsl:value-of select="$accessRights"/>
				</ddm:accessRights>
			</ddm:profile>
			<ddm:dcmiMetadata>
				<xsl:apply-templates select="$DC/*"/>
			</ddm:dcmiMetadata>
		</ddm:DDM>
	</xsl:template>
	
	<xsl:template match="@*">
		<xsl:copy/>
	</xsl:template>

	<xsl:template match="*">
		<xsl:element name="{name()}">
			<xsl:apply-templates select="@*|node()"/>
		</xsl:element>
	</xsl:template>
	
</xsl:stylesheet>
