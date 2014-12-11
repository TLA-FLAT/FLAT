<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:foxml="info:fedora/fedora-system:def/foxml#" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:lat="http://lat.mpi.nl/" xmlns:sx="java:nl.mpi.tla.saxon" exclude-result-prefixes="xs sx lat" version="2.0">

	<xsl:variable name="rec" select="/"/>

	<xsl:param name="rels-uri" select="'./relations.xml'"/>
	<xsl:variable name="rels-doc" select="document($rels-uri)"/>
	<xsl:key name="rels-from" match="relation" use="from"/>
	<xsl:key name="rels-to" match="relation" use="to"/>

	<xsl:param name="conversion-base" select="()"/>
	<xsl:param name="import-base" select="()"/>
	<xsl:param name="fox-base" select="'./fox'"/>

	<xsl:function name="cmd:lat">
		<xsl:param name="prefix"/>
		<xsl:param name="pid"/>
		<xsl:variable name="suffix" select="replace(replace(replace(replace($pid,'http://hdl.handle.net/','hdl:'),'@format=.+',''),'[^a-zA-Z0-9]','_'),'^hdl_','')"/>
		<xsl:variable name="length" select="min((string-length($suffix), (64 - string-length($prefix))))"/>
		<xsl:sequence select="concat($prefix,substring($suffix,string-length($suffix) - $length + 1))"/>
	</xsl:function>

	<xsl:template match="/">
		<xsl:variable name="fid" select="cmd:lat('lat:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
		<xsl:message>DBG: CMDI2FOX[<xsl:value-of select="$fid"/>]</xsl:message>
		<foxml:digitalObject VERSION="1.1" PID="{$fid}" xmlns:xsii="http://www.w3.org/2001/XMLSchema-instance" xsii:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
			<foxml:objectProperties>
				<!-- [A]ctive state -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#state" VALUE="A"/>
				<!-- take the first corpus or session title found (only works for IMDI-based data) -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#label" VALUE="{(//(cmd:Corpus|cmd:Session)/cmd:Title)[1]}"/>
			</foxml:objectProperties>
			<!-- Metadata: Dublin Core -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="X">
				<foxml:datastreamVersion ID="DC.0" FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml" LABEL="Dublin Core Record for this object">
					<foxml:xmlContent>
						<xsl:apply-templates mode="dc"/>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Metadata: CMDI -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="CMDI" STATE="A" CONTROL_GROUP="X">
				<foxml:datastreamVersion ID="CMDI.0" FORMAT_URI="{/cmd:CMD/@xsii:schemaLocation}" LABEL="CMDI Record for this object" MIMETYPE="text/xml">
					<foxml:xmlContent>
						<xsl:apply-templates mode="cmdi"/>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Relations: RELS-EXT -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="RELS-EXT" STATE="A" CONTROL_GROUP="X" VERSIONABLE="true">
				<foxml:datastreamVersion ID="RELS-EXT.0" LABEL="RDF Statements about this object" MIMETYPE="text/xml">
					<foxml:xmlContent>
						<rdf:RDF xmlns:oai="http://www.openarchives.org/OAI/2.0/" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
							<rdf:Description rdf:about="info:fedora/{$fid}">
								<!-- OAI -->
								<oai:itemID xmlns="http://www.openarchives.org/OAI/2.0/">
									<xsl:value-of select="cmd:lat('oai:lat.mpi.nl:',/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
								</oai:itemID>
								<!-- relationships to (parent) collections -->
								<xsl:variable name="parents" select="$rels-doc/key('rels-to',$rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)[type='Metadata']/from"/>
								<xsl:choose>
									<xsl:when test="exists($parents)">
										<xsl:for-each select="$parents">
											<fedora:isMemberOfCollection rdf:resource="info:fedora/{cmd:lat('lat:',current())}"/>
										</xsl:for-each>
									</xsl:when>
									<xsl:otherwise>
										<fedora:isMemberOfCollection rdf:resource="info:fedora/islandora:compound_collection"/>
									</xsl:otherwise>
								</xsl:choose>
								<!-- if the CMDI has references to other metadata files it's a collection -->
								<xsl:if test="exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Metadata'])">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:collectionCModel"/>
								</xsl:if>
								<!-- if the CMDI has references to resources it's a compound -->
								<xsl:if test="exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource'])">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:compoundCModel"/>
								</xsl:if>
							</rdf:Description>
						</rdf:RDF>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Resource Proxies -->
			<xsl:message>DBG: resourceProxies[<xsl:value-of select="count(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource'])"/>]</xsl:message>
			<xsl:for-each select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']">
				<xsl:message>DBG: resourceProxy[<xsl:value-of select="position()"/>][<xsl:value-of select="cmd:ResourceRef/@lat:localURI"/>]</xsl:message>
			</xsl:for-each>
			<xsl:for-each-group select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']" group-by="cmd:ResourceRef/@lat:localURI">
				<xsl:variable name="res" select="current-group()[1]"/>
				<xsl:variable name="resURI" select="resolve-uri($res/cmd:ResourceRef/@lat:localURI,base-uri())"/>
				<xsl:variable name="resFOX" select="concat($fox-base,'/',replace($resURI,'[^a-zA-Z0-9]','_'),'.xml')"/>
				<xsl:variable name="resPID" select="$res/cmd:ResourceRef"/>
				<xsl:variable name="resID" select="cmd:lat('lat:',$resPID)"/>
				<xsl:message>DBG: resourceProxy[<xsl:value-of select="$resURI"/>][<xsl:value-of select="$resFOX"/>][<xsl:value-of select="$resPID"/>][<xsl:value-of select="$resID"/>]</xsl:message>
				<!-- CHECK: take the filepart of the localURI as the resource title? -->
				<xsl:variable name="resTitle" select="replace($resURI,'.*/','')"/>
				<xsl:message>DBG: creating FOX[<xsl:value-of select="$resFOX"/>]?[<xsl:value-of select="not(doc-available($resFOX))"/>]</xsl:message>
				<xsl:choose>
					<xsl:when test="starts-with($resURI,'file:') and not(sx:fileExists($resURI))">
						<xsl:message>ERR: resource[<xsl:value-of select="$resURI"/>] linked from [<xsl:value-of select="base-uri()"/>] doesn't exist!</xsl:message>
					</xsl:when>
					<xsl:when test="not(doc-available($resFOX))">
						<xsl:message>DBG: creating resource FOX[<xsl:value-of select="$resFOX"/>]</xsl:message>
						<xsl:result-document href="{$resFOX}">
							<foxml:digitalObject VERSION="1.1" PID="{$resID}" xmlns:xsii="http://www.w3.org/2001/XMLSchema-instance" xsii:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
								<foxml:objectProperties>
									<!-- [A]ctive state -->
									<foxml:property NAME="info:fedora/fedora-system:def/model#state" VALUE="A"/>
									<foxml:property NAME="info:fedora/fedora-system:def/model#label" VALUE="{$resTitle}"/>
								</foxml:objectProperties>
								<!-- Metadata: Dublin Core -->
								<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="X">
									<foxml:datastreamVersion ID="DC.0" FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml" LABEL="Dublin Core Record for this object">
										<foxml:xmlContent>
											<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
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
											<rdf:RDF xmlns:oai="http://www.openarchives.org/OAI/2.0/" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
												<rdf:Description rdf:about="info:fedora/{$resID}">
													<!-- relationships to (parent) compounds -->
													<xsl:variable name="compounds" select="$rels-doc/key('rels-to',$resPID)[type='Resource']/from"/>
													<xsl:for-each select="$compounds">
														<fedora:isConstituentOf rdf:resource="info:fedora/{cmd:lat('lat:',current())}"/>
													</xsl:for-each>
													<!-- resource has to become a member of the collection the compound is a member of -->
													<xsl:variable name="collections" select="distinct-values(for $c in $compounds return $rels-doc/key('rels-to',$c)[type='Metadata']/from)"/>
													<xsl:choose>
														<xsl:when test="exists($collections)">
															<xsl:for-each select="$collections">
																<fedora:isMemberOfCollection rdf:resource="info:fedora/{cmd:lat('lat:',current())}"/>
															</xsl:for-each>
														</xsl:when>
														<xsl:otherwise>
															<fedora:isMemberOfCollection rdf:resource="info:fedora/islandora:compound_collection"/>
														</xsl:otherwise>
													</xsl:choose>
												</rdf:Description>
											</rdf:RDF>
										</foxml:xmlContent>
									</foxml:datastreamVersion>
								</foxml:datastream>
								<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="RESOURCE" STATE="A">
									<!--- CHECK: CONTROL_GROUP indicates the kind of datastream, either
									Externally Referenced Content (E), 
									Redirected Content (R), 
									Managed Content (M) or 
									Inline XML (X) -->
									<xsl:choose>
										<xsl:when test="starts-with($resURI,'file:')">
											<xsl:attribute name="CONTROL_GROUP" select="'E'"/>
										</xsl:when>
										<xsl:otherwise>
											<xsl:attribute name="CONTROL_GROUP" select="'R'"/>
										</xsl:otherwise>
									</xsl:choose>
									<foxml:datastreamVersion ID="RESOURCE.0" LABEL="{$resTitle}" MIMETYPE="{$res/cmd:ResourceType/@mimetype}">
										<foxml:contentLocation TYPE="URL">
											<xsl:choose>
												<xsl:when test="starts-with($resURI,'file:') and exists($import-base)">
													<xsl:attribute name="REF" select="replace($resURI,$conversion-base,$import-base)"/>
												</xsl:when>
												<xsl:otherwise>
													<xsl:attribute name="REF" select="$resURI"/>
												</xsl:otherwise>
											</xsl:choose>
										</foxml:contentLocation>
									</foxml:datastreamVersion>
								</foxml:datastream>
							</foxml:digitalObject>
						</xsl:result-document>
					</xsl:when>
				</xsl:choose>
			</xsl:for-each-group>
		</foxml:digitalObject>
	</xsl:template>

	<!-- CMDI -->

	<!-- identity copy -->
	<xsl:template match="@*|node()" mode="cmdi">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" mode="#current"/>
		</xsl:copy>
	</xsl:template>

	<!-- strip out the CMDI-invalid @lat:localURI -->
	<xsl:template match="@lat:*" priority="1" mode="cmdi">
		<xsl:attribute name="lat:{local-name()}" xmlns:lat="http://lat.mpi.nl/">
			<xsl:value-of select="cmd:lat('lat:',parent::cmd:ResourceRef)"/>
		</xsl:attribute>
	</xsl:template>

	<!-- Dublin Core -->

	<xsl:template match="cmd:CMD" mode="dc">
		<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
			<xsl:apply-templates select="cmd:Components/*/cmd:Title" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:*[not(local-name()='Title')]" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:MDGroup/cmd:Location/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Project/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Project/cmd:Contact/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Content/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Content/cmd:Languages/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Actors/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:Source/cmd:*" mode="#current"/>
			<xsl:apply-templates select="cmd:Components/*/cmd:References/cmd:*" mode="#current"/>
		</oai_dc:dc>
	</xsl:template>

	<xsl:template match="text()" mode="dc"/>

	<!-- Session|Corpus.Name -->
	<xsl:template match="cmd:Components/*/cmd:Name" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session|Corpus.Title -->
	<xsl:template match="cmd:Components/*/cmd:Title" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Date -->
	<xsl:template match="cmd:Components/*/cmd:Date" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:date</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Location.Country -->
	<xsl:template match="cmd:Components/*/cmd:Location/cmd:Country" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:coverage</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Title -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Title" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Id -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Id" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:identifier</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Contact.Name -->
	<xsl:template match="cmd:Components/*/cmd:Contact/cmd:Name" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:publisher</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Contact.Organisation -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Contact/cmd:Organisation" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:publisher</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.Genre -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:Genre" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.SubGenre -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:SubGenre" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.Languages.Language.* -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:Content_Languages/cmd:Content_Language" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="value-node" select="cmd:Name"/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Actors.Actor -->
	<xsl:template match="cmd:Components/*/cmd:Actors/cmd:Actor" mode="dc">
		<xsl:apply-templates select="cmd:Name" mode="#current"/>
		<xsl:apply-templates select="cmd:Description" mode="#current"/>
	</xsl:template>
	<!-- Session.Actors.Actor.Role -->
	<xsl:template match="cmd:Components/*/cmd:Actors/cmd:Actor/cmd:Name" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:contributor</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.MediaFile.Type -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:Type" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:type</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.MediaFile.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:Format" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:Format" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.ContentEncoding -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:ContentEncoding" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.CharacterEncoding -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:CharacterEncoding" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.Source.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:Source/cmd:Format" mode="dc">
		<xsl:call-template name="create-dc-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>

	<!-- Description (sub) -->
	<xsl:template match="cmd:Description" mode="dc">
		<xsl:if test="not(normalize-space(.)='')">
			<dc:description xmlns:dc="http://purl.org/dc/elements/1.1/">
				<xsl:copy-of select="@xml:lang"/>
				<xsl:value-of select="text()"/>
				<xsl:if test="not(@Link='')">
					<xsl:value-of select="@Link"/>
				</xsl:if>
			</dc:description>
		</xsl:if>
	</xsl:template>

	<!-- create dublin core element -->
	<xsl:template name="create-dc-element">
		<xsl:param name="dc-name"/>
		<xsl:param name="value-node"/>
		<xsl:if test="not(normalize-space($value-node)='') and (normalize-space($value-node)!='Unspecified')">
			<xsl:element name="{$dc-name}" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/">
				<xsl:value-of select="normalize-space($value-node)"/>
			</xsl:element>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>
