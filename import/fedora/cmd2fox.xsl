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
	
	<xsl:param name="rels-uri" select="'./relations.xml'"/>
	<xsl:variable name="rels-doc" select="document($rels-uri)"/>
	<xsl:key name="rels-from" match="relation" use="from"/>
	<xsl:key name="rels-to" match="relation" use="to"/>
	
	<xsl:function name="cmd:tla">
		<xsl:param name="prefix"/>
		<xsl:param name="pid"/>
		<xsl:variable name="suffix" select="replace(replace(replace(replace($pid,'http://hdl.handle.net/','hdl:'),'@format=.+',''),'[^a-zA-Z0-9]','_'),'^hdl_','')"/>
		<xsl:variable name="length" select="min((string-length($suffix), (64 - string-length($prefix))))"/>
		<xsl:sequence select="concat($prefix,substring($suffix,string-length($suffix) - $length + 1))"/>
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
						<xsl:apply-templates select="doc(replace(base-uri(),'.cmdi','.dc'))/*"/>
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
							<rdf:Description rdf:about="{cmd:tla('tla:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)}">
								<oai:itemID xmlns="http://www.openarchives.org/OAI/2.0/">
									<xsl:value-of select="cmd:tla('oai:tla.mpi.nl:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
								</oai:itemID>
							</rdf:Description>
							<!-- relationships to (parent) collections -->
							<xsl:variable name="rels" select="$rels-doc/key('rels-to',$rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
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
									<fedora:isMemberOfCollection rdf:resource="{cmd:tla('tla:',current())}"/>
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
			<xsl:for-each-group select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']" group-by="@lcl:localURI">
				<xsl:variable name="res" select="current-group()[1]"/>
				<xsl:variable name="resURI" select="resolve-uri($res/cmd:ResourceRef/@lcl:localURI,base-uri())"/>
				<xsl:variable name="resFOX" select="concat($resURI,'.fox')"/>
				<xsl:variable name="resPID" select="$res/cmd:ResourceRef"/>
				<xsl:variable name="resID" select="cmd:tla('tla:',$resPID)"/>
				<xsl:choose>
					<xsl:when test="starts-with($resURI,'file:')">
						<!-- CHECK: take the filepart of the localURI as the resource title? -->
						<xsl:variable name="resTitle" select="replace($resURI,'.*/','')"/>
						<xsl:if test="not(doc-available($resFOX))">
							<xsl:message>DBG: creating resource FOX[<xsl:value-of select="$resFOX"/>]</xsl:message>
							<xsl:result-document href="{$resFOX}">
								<foxml:digitalObject 
									VERSION="1.1" PID="{$resID}" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
									xsi:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
									<foxml:objectProperties>
										<!-- [A]ctive state -->
										<foxml:property NAME="info:fedora/fedora-system:def/model#state"
											VALUE="A" />
										<foxml:property NAME="info:fedora/fedora-system:def/model#label"
											VALUE="{$resTitle}" />
									</foxml:objectProperties>
									<!-- Metadata: Dublin Core -->
									<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="E">
										<foxml:datastreamVersion ID="DC.0"
											FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml"
											LABEL="Dublin Core Record for this object">
											<foxml:xmlContent>
												<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
													<dc:title>
														<xsl:value-of select="$resTitle"/>
													</dc:title>
												</oai_dc:dc>
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
													<rdf:Description rdf:about="{$resPID}">
														<oai:itemID xmlns="http://www.openarchives.org/OAI/2.0/">
															<xsl:value-of select="cmd:tla('oai:tla.mpi.nl:',$resPID)"/>
														</oai:itemID>
													</rdf:Description>
													<!-- relationships to (parent) compounds -->
													<xsl:variable name="compounds" select="$rels-doc/key('rels-to',$resPID)[type='Resource']"/>
													<xsl:for-each select="$compounds">
														<rdf:Description rdf:about="{$resPID}">
															<fedora:isConstituentOf rdf:resource="{cmd:tla('tla:',current())}"/>
														</rdf:Description>
													</xsl:for-each>
													<!-- resource has to become a member of the collection the compound is a member of -->
													<xsl:variable name="collections" select="distinct-values(for $c in $compounds return $rels-doc/key('rels-to',$c)[type='Metadata']/@from)"/>
													<xsl:choose>
														<xsl:when test="exists($collections)">
															<rdf:Description rdf:about="{$resID}">
																<fedora:isMemberOfCollection rdf:resource="{cmd:tla('tla:',current())}"/>
															</rdf:Description>
														</xsl:when>
														<xsl:otherwise>
															<rdf:Description rdf:about="{$resID}">
																<fedora:isMemberOfCollection rdf:resource="info:fedora/islandora:compound_collection"/>
															</rdf:Description>
														</xsl:otherwise>
													</xsl:choose>
												</rdf:RDF>
											</foxml:xmlContent>
										</foxml:datastreamVersion>
									</foxml:datastream>
									<!--- CHECK: CONTROL_GROUP indicates the kind of datastream, either Externally Referenced Content (E), Redirected Content (R), Managed Content (M) or Inline XML (X) -->
									<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="RESOURCE" STATE="A" CONTROL_GROUP="M">
										<foxml:datastreamVersion ID="RESOURCE.0" MIMETYPE="{$res/cmd:ResourceType/@mimetype}">
											<foxml:contentLocation REF="{$res}" TYPE="URL"/>
										</foxml:datastreamVersion>
									</foxml:datastream>
								</foxml:digitalObject>
							</xsl:result-document>
						</xsl:if>
					</xsl:when>
					<xsl:otherwise>
						<xsl:message>WRN: local URI[<xsl:value-of select="$resURI"/>] for ResourceProxy[<xsl:value-of select="@id"/>][<xsl:value-of select="$resPID"/>] isn't a file: URI!</xsl:message>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each-group>
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