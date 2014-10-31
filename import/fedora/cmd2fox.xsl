<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
	xmlns:foxml="info:fedora/fedora-system:def/foxml#"
	xmlns:cmd="http://www.clarin.eu/cmd/"
	xmlns:lcl="http://www.clarin.eu/cmd/extension"
	exclude-result-prefixes="xs"
	version="2.0">
	
	<xsl:variable name="rec" select="/"/>
	
	<xsl:function name="cmd:tla">
		<xsl:param name="prefix"/>
		<xsl:param name="pid"/>
		<xsl:sequence select="replace(replace(replace(replace($pid,'http://hdl.handle.net/','hdl:'),'@format=.+',''),'[^a-z0-9]','_'),'^hdl_',$prefix)"/>
	</xsl:function>
	
	<xsl:template match="/">
		<foxml:digitalObject 
			VERSION="1.1" PID="{cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)}" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
			xsi:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
			<foxml:objectProperties>
				<!-- [A]ctive state -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#state"
					VALUE="A" />
				<!-- take the first corpus or session title found (only works for IMDI-based data) -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#label"
					VALUE="{(//(cmd:Corpus|cmd:Session)/cmd:Title)[1]}" />
			</foxml:objectProperties>
			<!-- Metadata: Dublin Core -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="E">
				<foxml:datastreamVersion ID="DC.0"
					FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml"
					LABEL="Dublin Core Record for this object">
					<foxml:xmlContent>
						<xsl:apply-templates select="doc(replace(base-uri(),'.imdi','.dc'))/*"/>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Metadata: CMDI -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="CMDI" STATE="A" CONTROL_GROUP="E">
				<foxml:datastreamVersion ID="CMDI.0"
					FORMAT_URI="{/cmd:CMD/@xsi:schemaLocation}"
					LABEL="CMDI Record for this object"
					MIMETYPE="text/xml">
					<foxml:xmlContent>
						<xsl:apply-templates select="*"/>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Relations: RELS-EXT -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="RELS-EXT" STATE="A" CONTROL_GROUP="X" VERSIONABLE="true">
				<foxml:datastreamVersion ID="RELS-EXT.0" LABEL="RDF Statements about this object" MIMETYPE="text/xml">
					<foxml:xmlContent>
						<rdf:RDF xmlns:oai="http://www.openarchives.org/OAI/2.0/"
							xmlns:fedora="info:fedora/fedora-system:def/relations-external#"
							xmlns:fedora-model="info:fedora/fedora-system:def/model#"
							xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
							<rdf:Description>
								<xsl:attribute name="rdf:about">
									<xsl:value-of select="cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
								</xsl:attribute>
								<oai:itemID xmlns="http://www.openarchives.org/OAI/2.0/">
									<xsl:value-of select="cmd:tla('oai:tla.mpi.nl:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
								</oai:itemID>
							</rdf:Description>
							<!-- relationships to (parent) collections -->
							<xsl:variable name="rels" select="document('file:/Users/menzowindhouwer/Documents/Projects/LAT-fedora/test/relations.xml')//relation[to=$rec/cmd:CMD/cmd:Header/cmd:MdSelfLink]"/>
							<xsl:variable name="parents">
								<xsl:choose>
									<xsl:when test="exists($rels)">
										<xsl:sequence select="$rels[type='Metadata']/from"/>
									</xsl:when>
									<xsl:otherwise>
										<xsl:sequence select="'info:fedora/islandora:compound_collection'"/>
									</xsl:otherwise>
								</xsl:choose>
							</xsl:variable>
							<xsl:for-each select="$parents">
								<rdf:Description rdf:about="{cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)}">
									<fedora:isMemberOfCollection rdf:resource="{current()}"/>
								</rdf:Description>
							</xsl:for-each>
							<!-- if the CMDI has references to other metadata files it's a collection -->
							<xsl:if test="exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Metadata'])">
								<rdf:Description rdf:about="{cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)}">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:collectionCModel"/>
								</rdf:Description>
							</xsl:if>
							<!-- if the CMDI has references to resources it's a compound -->
							<xsl:if test="exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource'])">
								<rdf:Description rdf:about="{cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)}">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:compoundCModel"/>
								</rdf:Description>
							</xsl:if>
						</rdf:RDF>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Resource Proxies -->
			<xsl:for-each select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']">
				<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="{@id}" STATE="A" CONTROL_GROUP="R">
					<foxml:datastreamVersion ID="{@id}.0" MIMETYPE="{cmd:ResourceType/@mimetype}">
						<foxml:contentLocation REF="{resolve-uri(base-uri(),cmd:ResourceRef/@lcl:localURI)}" TYPE="URL"/>
					</foxml:datastreamVersion>
				</foxml:datastream>
			</xsl:for-each>
		</foxml:digitalObject>
	</xsl:template>
	
	<!-- identity copy -->
	<xsl:template match="@*|node()">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()"/>
		</xsl:copy>
	</xsl:template>
	
	<!-- strip out the CMDI-invalid @lcl:localURI -->
	<xsl:template match="@lcl:*" priority="1"/>

</xsl:stylesheet>