<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:foxml="info:fedora/fedora-system:def/foxml#" xmlns:cmd="http://www.clarin.eu/cmd/" xmlns:lat="http://lat.mpi.nl/" xmlns:sx="java:nl.mpi.tla.saxon" exclude-result-prefixes="xs sx lat" version="2.0">

	<xsl:variable name="rec" select="/"/>

	<xsl:param name="rels-uri" select="'./relations.xml'"/>
	<xsl:param name="rels-doc" select="document($rels-uri)"/>
	<xsl:key name="rels-from" match="relation" use="src|from"/>
	<xsl:key name="rels-to" match="relation" use="dst|to"/>

	<xsl:param name="conversion-base" select="()"/>
	<xsl:param name="import-base" select="()"/>
	<xsl:param name="fox-base" select="'./fox'"/>
	<xsl:param name="lax-resource-check" select="false()"/>
	
	<xsl:param name="create-cmd-object" select="true()"/>
	
	<xsl:function name="cmd:hdl">
		<xsl:param name="pid"/>
		<xsl:sequence select="replace(replace($pid,'^http://hdl.handle.net/','hdl:'),'@format=[a-z]+','')"/>
	</xsl:function>

	<xsl:function name="cmd:lat">
		<xsl:param name="prefix"/>
		<xsl:param name="pid"/>
		<xsl:variable name="suffix" select="replace(replace(cmd:hdl($pid),'[^a-zA-Z0-9]','_'),'^hdl_','')"/>
		<xsl:variable name="length" select="min((string-length($suffix), (64 - string-length($prefix))))"/>
		<xsl:sequence select="concat($prefix,substring($suffix,string-length($suffix) - $length + 1))"/>
	</xsl:function>
	
	<xsl:function name="cmd:pid">
		<xsl:param name="locs"/>
                <xsl:param name="lvl"/>
		<xsl:variable name="to" select="$rels-doc/key('rels-to',for $l in $locs return cmd:hdl($l))"/>
		<xsl:variable name="from" select="$rels-doc/key('rels-from',for $l in $locs return cmd:hdl($l))"/>
		<xsl:variable name="refs" select="distinct-values(($from/src,$from/from,$to/dst,$to/to))[normalize-space(.)!='']"/>
		<xsl:variable name="hdl" select="$refs[starts-with(.,'hdl:')]"/>
		<xsl:choose>
			<xsl:when test="count($hdl) eq 0">
				<xsl:message><xsl:value-of select="$lvl"/>: the handle for resource[<xsl:value-of select="string-join(distinct-values(for $l in $locs return cmd:hdl($l)),', ')"/>][<xsl:value-of select="string-join($refs,', ')"/>] can't be determined!</xsl:message>
				<xsl:sequence select="()"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:if test="count($hdl) gt 1">
					<xsl:message>ERR: there are multiple handles[<xsl:value-of select="string-join($hdl,', ')"/>] for resource[<xsl:value-of select="string-join(distinct-values(for $l in $locs return cmd:hdl($l)),', ')"/>][<xsl:value-of select="string-join($refs,', ')"/>]! Using the first one ...</xsl:message>
				</xsl:if>
				<xsl:sequence select="cmd:hdl(($hdl)[1])"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:function>
        
        <xsl:function name="cmd:pid">
            <xsl:param name="locs"/>
            <xsl:sequence select="cmd:pid($locs,'ERR')"/>
        </xsl:function>

	<xsl:template match="/">
		<xsl:variable name="base" select="base-uri()"/>
		<xsl:variable name="self" select="normalize-space($rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
		<xsl:variable name="pid">
			<xsl:choose>
				<xsl:when test="normalize-space($rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)=''">
					<xsl:message>ERR: CMD record[<xsl:value-of select="$base"/>] has no or empty MdSelfLink!</xsl:message>
					<xsl:variable name="hdl" select="cmd:pid($base)"/>
					<xsl:choose>
						<xsl:when test="normalize-space($hdl)!=''">
							<xsl:message>WRN: found handle[<xsl:value-of select="$hdl"/>] for CMD record[<xsl:value-of select="$base"/>]</xsl:message>
							<xsl:sequence select="$hdl"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:message>WRN: using base URI instead.</xsl:message>
							<xsl:sequence select="$base"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="starts-with(cmd:hdl($rec/cmd:CMD/cmd:Header/cmd:MdSelfLink),'hdl:')">
					<xsl:sequence select="cmd:hdl($rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:sequence select="normalize-space($rec/cmd:CMD/cmd:Header/cmd:MdSelfLink)"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="fid"  select="cmd:lat('lat:',$pid)"/>
		<xsl:message>DBG: CMDI2FOX[<xsl:value-of select="$pid"/>][<xsl:value-of select="$fid"/>]</xsl:message>
		<!--<xsl:message>DBG: [<xsl:value-of select="$rels-doc/key('rels-from',$pid)[1]/src"/>]!=[<xsl:value-of select="base-uri()"/>] => [<xsl:value-of select="$rels-doc/key('rels-from',$pid)[1]/src!=base-uri()"/>]</xsl:message>-->
		<xsl:if test="$rels-doc/key('rels-from',$pid)[1]/src!=$base">
			<xsl:message>ERR: record[<xsl:value-of select="$base"/>] has an already used PID URI[<xsl:value-of select="$pid"/>][<xsl:value-of select="(key('rels-from',$pid))[1]/src"/>]!</xsl:message>
			<xsl:message terminate="yes">WRN: resource FOX[<xsl:value-of select="$fid"/>] will not be created!</xsl:message>
		</xsl:if>
		<xsl:variable name="dc">
			<xsl:apply-templates mode="dc"/>
		</xsl:variable>
		<foxml:digitalObject VERSION="1.1" PID="{$fid}" xmlns:xsii="http://www.w3.org/2001/XMLSchema-instance" xsii:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
			<xsl:comment>
				<xsl:text>Source: </xsl:text>
				<xsl:value-of select="base-uri()"/>
			</xsl:comment>
			<foxml:objectProperties>
				<!-- [A]ctive state -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#state" VALUE="A"/>
				<!-- take the first title found in the Dublin Core -->
				<foxml:property NAME="info:fedora/fedora-system:def/model#label">
					<xsl:variable name="label" select="($dc//dc:title[normalize-space()!=''])[1]" xmlns:dc="http://purl.org/dc/elements/1.1/"/>
					<xsl:choose>
						<xsl:when test="exists($label)">
							<xsl:attribute name="VALUE" select="substring($label,1,255)"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:attribute name="VALUE" select="$pid"/>
						</xsl:otherwise>
					</xsl:choose>
				</foxml:property>
			</foxml:objectProperties>
			<!-- Metadata: Dublin Core -->
			<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="X">
				<foxml:datastreamVersion ID="DC.0" FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml" LABEL="Dublin Core Record for this object">
					<foxml:xmlContent>
						<xsl:copy-of select="$dc"/>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Metadata: CMD -->
			<xsl:choose>
				<xsl:when test="$create-cmd-object">
					<xsl:variable name="cmdID" select="cmd:lat('lat:',concat($pid,'-CMD'))"/>
					<xsl:variable name="cmdFOX" select="concat($fox-base,'/',replace($cmdID,'[^a-zA-Z0-9]','_'),'.xml')"/>
					<xsl:choose>
						<xsl:when test="not(doc-available($cmdFOX))">
							<xsl:message>DBG: creating CMD FOX[<xsl:value-of select="$cmdFOX"/>]</xsl:message>
							<xsl:result-document href="{$cmdFOX}">
								<foxml:digitalObject VERSION="1.1" PID="{$cmdID}" xmlns:xsii="http://www.w3.org/2001/XMLSchema-instance" xsii:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
									<xsl:comment>
									<xsl:text>Source: </xsl:text>
									<xsl:value-of select="base-uri()"/>
								</xsl:comment>
									<foxml:objectProperties>
										<!-- [A]ctive state -->
										<foxml:property NAME="info:fedora/fedora-system:def/model#state" VALUE="A"/>
										<!-- take the first title found in the Dublin Core -->
										<foxml:property NAME="info:fedora/fedora-system:def/model#label">
											<xsl:variable name="label" select="($dc//dc:title[normalize-space()!=''])[1]" xmlns:dc="http://purl.org/dc/elements/1.1/"/>
											<xsl:choose>
												<xsl:when test="exists($label)">
													<xsl:attribute name="VALUE" select="substring($label,1,255)"/>
												</xsl:when>
												<xsl:otherwise>
													<xsl:attribute name="VALUE" select="$pid"/>
												</xsl:otherwise>
											</xsl:choose>
										</foxml:property>
									</foxml:objectProperties>
									<!-- Metadata: Dublin Core -->
									<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="DC" STATE="A" CONTROL_GROUP="X">
										<foxml:datastreamVersion ID="DC.0" FORMAT_URI="http://www.openarchives.org/OAI/2.0/oai_dc/" MIMETYPE="text/xml" LABEL="Dublin Core Record for this object">
											<foxml:xmlContent>
												<xsl:copy-of select="$dc"/>
											</foxml:xmlContent>
										</foxml:datastreamVersion>
									</foxml:datastream>
									<!-- Relations: RELS-EXT -->
									<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="RELS-EXT" STATE="A" CONTROL_GROUP="X" VERSIONABLE="true">
										<foxml:datastreamVersion ID="RELS-EXT.0" LABEL="RDF Statements about this object" MIMETYPE="text/xml">
											<foxml:xmlContent>
												<rdf:RDF xmlns:oai="http://www.openarchives.org/OAI/2.0/" xmlns:fedora="info:fedora/fedora-system:def/relations-external#" xmlns:fedora-model="info:fedora/fedora-system:def/model#" xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#" xmlns:islandora="http://islandora.ca/ontology/relsext#">
													<rdf:Description rdf:about="info:fedora/{$cmdID}">
														<!-- relationship to the compound -->
														<fedora:isConstituentOf rdf:resource="info:fedora/{$fid}"/>
														<!-- make it the first object in the compound -->
														<xsl:element name="{concat('islandora:isSequenceNumberOf',replace($fid,':','_'))}">
															<xsl:text>1</xsl:text>
														</xsl:element>
														<!-- CMD object has to become a member of the collections the compound is a member of -->
														<xsl:variable name="parents" select="distinct-values($rels-doc/key('rels-to',($pid,$base))[type='Metadata']/from)"/>
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
													</rdf:Description>
												</rdf:RDF>
											</foxml:xmlContent>
										</foxml:datastreamVersion>
									</foxml:datastream>
									<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="CMD" STATE="A" CONTROL_GROUP="X">
										<foxml:datastreamVersion ID="CMD.0" FORMAT_URI="{/cmd:CMD/@xsii:schemaLocation}" LABEL="CMD Record for this object" MIMETYPE="text/xml">
											<foxml:xmlContent>
												<xsl:apply-templates mode="cmd">
													<xsl:with-param name="pid" select="$pid" tunnel="yes"/>
													<xsl:with-param name="base" select="$base" tunnel="yes"/>
												</xsl:apply-templates>
											</foxml:xmlContent>
										</foxml:datastreamVersion>
									</foxml:datastream>
								</foxml:digitalObject>
							</xsl:result-document>
						</xsl:when>
						<xsl:otherwise>
							<xsl:message>WRN: skipped creating CMD FOX[<xsl:value-of select="$cmdFOX"/>], it already exists!</xsl:message>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:otherwise>
					<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="CMD" STATE="A" CONTROL_GROUP="X">
						<foxml:datastreamVersion ID="CMD.0" FORMAT_URI="{/cmd:CMD/@xsii:schemaLocation}" LABEL="CMD Record for this object" MIMETYPE="text/xml">
							<foxml:xmlContent>
								<xsl:apply-templates mode="cmd">
									<xsl:with-param name="pid" select="$pid" tunnel="yes"/>
									<xsl:with-param name="base" select="$base" tunnel="yes"/>
								</xsl:apply-templates>
							</foxml:xmlContent>
						</foxml:datastreamVersion>
					</foxml:datastream>
				</xsl:otherwise>
			</xsl:choose>
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
								<!--<xsl:message>DBG: look for parents (rels-to:dst|to) of [<xsl:value-of select="$pid"/>] or [<xsl:value-of select="$base"/>] </xsl:message>-->
								<xsl:variable name="parents" select="distinct-values($rels-doc/key('rels-to',($pid,$base))[type='Metadata']/from)"/>
								<!--<xsl:message>DBG: parents[<xsl:value-of select="string-join($parents,', ')"/>] </xsl:message>-->
								<xsl:choose>
									<xsl:when test="exists($parents)">
										<xsl:for-each select="$parents">
											<fedora:isMemberOfCollection rdf:resource="info:fedora/{cmd:lat('lat:',current())}"/>
										</xsl:for-each>
									</xsl:when>
									<xsl:otherwise>
										<!--<xsl:message>DBG: NO parents[<xsl:value-of select="string-join($parents,', ')"/>] </xsl:message>-->
										<fedora:isMemberOfCollection rdf:resource="info:fedora/islandora:compound_collection"/>
									</xsl:otherwise>
								</xsl:choose>
								<!-- if the CMD has references to other metadata files it's a collection -->
								<xsl:if test="exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Metadata'])">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:collectionCModel"/>
								</xsl:if>
								<!-- if the CMD is a separate object and/or the CMD has references to resources it's a compound -->
								<xsl:if test="$create-cmd-object or exists(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource'])">
									<fedora-model:hasModel rdf:resource="info:fedora/islandora:compoundCModel"/>
								</xsl:if>
							</rdf:Description>
						</rdf:RDF>
					</foxml:xmlContent>
				</foxml:datastreamVersion>
			</foxml:datastream>
			<!-- Resource Proxies -->
			<!--<xsl:message>DBG: resourceProxies[<xsl:value-of select="count(/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource'])"/>]</xsl:message>
			<xsl:for-each select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']">
				<xsl:message>DBG: resourceProxy[<xsl:value-of select="position()"/>][<xsl:value-of select="cmd:ResourceRef"/>][<xsl:value-of select="cmd:ResourceRef/@lat:localURI"/>]</xsl:message>
			</xsl:for-each>-->
			<xsl:for-each-group select="/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']" group-by="cmd:ResourceRef/normalize-space(@lat:localURI)">
				<xsl:for-each select="if (current-grouping-key()='') then (current-group()) else (current-group()[1])">
					<xsl:variable name="res" select="current()"/>
					<xsl:variable name="resPID">
						<xsl:choose>
							<xsl:when test="starts-with(cmd:hdl($res/cmd:ResourceRef),'hdl:')">
								<xsl:sequence select="cmd:hdl($res/cmd:ResourceRef)"/>
							</xsl:when>
							<xsl:otherwise>
								<xsl:sequence select="resolve-uri($res/cmd:ResourceRef,$base)"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:variable>
					<xsl:variable name="resURI">
						<xsl:choose>
							<xsl:when test="normalize-space($res/cmd:ResourceRef/@lat:localURI)!=''">
								<xsl:sequence select="resolve-uri($res/cmd:ResourceRef/@lat:localURI,$base)"/>
							</xsl:when>
							<xsl:when test="normalize-space($resPID)!=''">
								<xsl:sequence select="resolve-uri($resPID,$base)"/>
							</xsl:when>
						</xsl:choose>
					</xsl:variable>
					<xsl:variable name="resID" select="cmd:lat('lat:',$resPID)"/>
					<!--<xsl:variable name="resFOX" select="concat($fox-base,'/',replace($resURI,'[^a-zA-Z0-9]','_'),'.xml')"/>-->
					<xsl:variable name="resFOX" select="concat($fox-base,'/',replace($resID,'[^a-zA-Z0-9]','_'),'.xml')"/>
					<!--<xsl:message>DBG: resourceProxy[<xsl:value-of select="$resURI"/>][<xsl:value-of select="$resFOX"/>][<xsl:value-of select="$resPID"/>][<xsl:value-of select="$resID"/>]</xsl:message>-->
					<!-- take the filepart of the localURI as the resource title -->
					<xsl:variable name="resTitle" select="replace($resURI,'.*/','')"/>
					<!--<xsl:message>DBG: creating FOX[<xsl:value-of select="$resFOX"/>]?[<xsl:value-of select="not(doc-available($resFOX))"/>]</xsl:message>-->
					<xsl:variable name="createFOX" as="xs:boolean">
						<xsl:choose>
							<xsl:when test="(key('rels-to',$resPID))[1]/resolve-uri(dst,src)!=$resURI">
								<xsl:message>ERR: resource[<xsl:value-of select="$resURI"/>] has an already used PID URI[<xsl:value-of select="$resPID"/>][<xsl:value-of select="(key('rels-to',$resPID))[1]/resolve-uri(dst,src)"/>]!</xsl:message>
								<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will not be created!</xsl:message>
								<xsl:sequence select="false()"/>
							</xsl:when>
							<xsl:when test="exists(key('rels-from',$resPID)[Type='Resource'])">
								<xsl:message>ERR: resource[<xsl:value-of select="$resURI"/>] has a PID[<xsl:value-of select="$resPID"/>] already used by one or more CMDI records[<xsl:value-of select="string-join(key('rels-from',$resPID)[Type='Resource']/src,', ')"/>]!</xsl:message>
								<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will not be created!</xsl:message>
								<xsl:sequence select="false()"/>
							</xsl:when>
							<xsl:when test="not(sx:checkURL(replace($resPID,'^hdl:','http://hdl.handle.net/')))">
								<xsl:message>ERR: resource[<xsl:value-of select="$resURI"/>] has an invalid PID URI[<xsl:value-of select="$resPID"/>]!</xsl:message>
								<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will not be created!</xsl:message>
								<xsl:sequence select="false()"/>
							</xsl:when>
							<xsl:when test="normalize-space($resURI)=''">
								<xsl:message>ERR: resource URI is empty, i.e., unknown!</xsl:message>
								<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will not be created!</xsl:message>
								<xsl:sequence select="false()"/>
							</xsl:when>
							<xsl:when test="starts-with($resURI,'file:') and exists($import-base) and sx:fileExists(replace($resURI,$conversion-base,$import-base))">
								<xsl:sequence select="true()"/>
							</xsl:when>
							<xsl:when test="starts-with($resURI,'file:') and not(sx:fileExists($resURI))">
								<xsl:variable name="uri">
									<xsl:choose>
										<xsl:when test="exists($import-base)">
											<xsl:sequence select="replace($resURI,$conversion-base,$import-base)"/>
										</xsl:when>
										<xsl:otherwise>
											<xsl:sequence select="$resURI"/>
										</xsl:otherwise>
									</xsl:choose>
								</xsl:variable>
								<xsl:choose>
									<xsl:when test="$lax-resource-check">
										<xsl:message>WRN: resource[<xsl:value-of select="$uri"/>] linked from [<xsl:value-of select="base-uri()"/>] doesn't exist!</xsl:message>
										<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will be created anyway</xsl:message>
										<xsl:sequence select="true()"/>
									</xsl:when>
									<xsl:otherwise>
										<xsl:message>ERR: resource[<xsl:value-of select="$uri"/>] linked from [<xsl:value-of select="base-uri()"/>] doesn't exist!</xsl:message>
										<xsl:message>WRN: resource FOX[<xsl:value-of select="$resFOX"/>] will not be created!</xsl:message>
										<xsl:sequence select="false()"/>
									</xsl:otherwise>
								</xsl:choose>
							</xsl:when>
							<xsl:otherwise>
								<xsl:sequence select="true()"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:variable>
					<xsl:if test="$createFOX and not(doc-available($resFOX))">
						<xsl:message>DBG: creating resource FOX[<xsl:value-of select="$resFOX"/>]</xsl:message>
						<xsl:result-document href="{$resFOX}">
							<foxml:digitalObject VERSION="1.1" PID="{$resID}" xmlns:xsii="http://www.w3.org/2001/XMLSchema-instance" xsii:schemaLocation="info:fedora/fedora-system:def/foxml# http://www.fedora.info/definitions/1/0/foxml1-1.xsd">
								<xsl:comment>
									<xsl:text>Source: </xsl:text>
									<xsl:value-of select="$resURI"/>
								</xsl:comment>
								<foxml:objectProperties>
									<!-- [A]ctive state -->
									<foxml:property NAME="info:fedora/fedora-system:def/model#state" VALUE="A"/>
									<foxml:property NAME="info:fedora/fedora-system:def/model#label" VALUE="{substring($resTitle,1,255)}"/>
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
													<xsl:variable name="compounds" select="distinct-values($rels-doc/key('rels-to',($resPID,$resURI))[type='Resource']/from)"/>
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
									<foxml:datastreamVersion ID="RESOURCE.0" LABEL="{substring($resTitle,1,255)}" MIMETYPE="{$res/cmd:ResourceType/@mimetype}">
										<foxml:contentLocation TYPE="URL">
											<xsl:choose>
												<xsl:when test="starts-with($resURI,'file:') and exists($import-base)">
													<xsl:attribute name="REF" select="replace($resURI,$conversion-base,$import-base)"/>
												</xsl:when>
												<xsl:otherwise>
													<xsl:attribute name="REF" select="replace($resURI,'^hdl:','http://hdl.handle.net/')"/>
												</xsl:otherwise>
											</xsl:choose>
										</foxml:contentLocation>
									</foxml:datastreamVersion>
								</foxml:datastream>
							</foxml:digitalObject>
						</xsl:result-document>
					</xsl:if>
				</xsl:for-each>
			</xsl:for-each-group>
		</foxml:digitalObject>
	</xsl:template>

	<!-- CMDI -->

	<!-- identity copy -->
	<xsl:template match="@*|node()" mode="cmd">
		<xsl:copy>
			<xsl:apply-templates select="@*|node()" mode="#current"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="cmd:Header" mode="cmd">
		<xsl:param name="pid" tunnel="yes"/>
		<xsl:copy>
			<xsl:apply-templates select="@*" mode="#current"/>
			<xsl:apply-templates select="cmd:MdCreator|cmd:MdCreationDate" mode="#current"/>
			<MdSelfLink xmlns="http://www.clarin.eu/cmd/">
				<xsl:copy-of select="cmd:MdSelfLink/@* except @lat:localURI"/>
				<xsl:attribute name="lat:localURI" select="cmd:lat('lat:',$pid)"/>
				<xsl:value-of select="$pid"/>
			</MdSelfLink>
			<xsl:apply-templates select="node() except (cmd:MdCreator|cmd:MdCreationDate|cmd:MdSelfLink)" mode="#current"/>
		</xsl:copy>
	</xsl:template>
	
	<xsl:template match="cmd:ResourceRef" mode="cmd">
		<xsl:param name="base" tunnel="yes"/>
		<xsl:variable name="pid">
			<xsl:choose>
				<xsl:when test="normalize-space(.)=''">
					<xsl:sequence select="()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:sequence select="resolve-uri(.,$base)"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:variable name="lcl">
			<xsl:choose>
				<xsl:when test="normalize-space(@lat:localURI)=''">
					<xsl:sequence select="()"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:sequence select="resolve-uri(@lat:localURI,$base)"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
                <xsl:variable name="lvl">
                    <xsl:choose>
                        <xsl:when test="../cmd:ResourceType=('Metadata','Resource')">
                            <xsl:sequence select="'ERR'"/>
                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:sequence select="'WRN'"/>
                        </xsl:otherwise>
                    </xsl:choose>
                </xsl:variable>
		<xsl:variable name="hdl" select="cmd:pid(($pid,$lcl),$lvl)"/>
		<xsl:choose>
			<xsl:when test="starts-with($pid,'file:') or starts-with($lcl,'file:')">
				<xsl:choose>
					<xsl:when test="empty($hdl)">
						<xsl:copy>
							<xsl:apply-templates select="@*|node()" mode="#current"/>
						</xsl:copy>
					</xsl:when>
					<xsl:otherwise>
						<xsl:copy>
							<xsl:apply-templates select="@* except @lat:localURI" mode="#current"/>
							<xsl:attribute name="lat:localURI" xmlns:lat="http://lat.mpi.nl/">
								<xsl:value-of select="cmd:lat('lat:',$hdl)"/>
							</xsl:attribute>
							<xsl:value-of select="$hdl"/>
						</xsl:copy>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:when>
			<xsl:otherwise>
				<xsl:copy>
					<xsl:apply-templates select="@*" mode="#current"/>
					<xsl:choose>
						<xsl:when test="empty($hdl)">
							<xsl:apply-templates select="node()" mode="#current"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:value-of select="$hdl"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:copy>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- turn @lat:localURI into a lat: reference -->
	<xsl:template match="@lat:localURI" priority="1" mode="cmd">
		<xsl:attribute name="lat:localURI" xmlns:lat="http://lat.mpi.nl/">
			<xsl:value-of select="cmd:lat('lat:',parent::cmd:ResourceRef)"/>
		</xsl:attribute>
	</xsl:template>

	<!-- Dublin Core -->
	<xsl:template match="cmd:CMD" mode="dc">
		<oai_dc:dc xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.openarchives.org/OAI/2.0/oai_dc/ http://www.openarchives.org/OAI/2.0/oai_dc.xsd">
			<xsl:variable name="dc">
				<xsl:apply-templates select="cmd:Components/*/cmd:Name" mode="#current"/>
				<xsl:apply-templates select="cmd:Components/*/cmd:Title" mode="#current"/>
				<xsl:apply-templates select="cmd:Components/*/cmd:*[not(local-name()=('Title','Name'))]" mode="#current"/>
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
			</xsl:variable>
			<!-- hard coded some fallbacks (created using the VLO-based mapping tool created for DASISH ;-) for non-IMDI-based CMDI profiles
				TODO: make this more generic -->
			<xsl:if test="empty($dc/dc:title[normalize-space()!=''])">
				<xsl:variable name="title" select="
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1328259700928')]/cmd:Components/cmd:Soundbites-recording/cmd:SESSION/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1328259700928')]/cmd:Components/cmd:Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:WrittenResource/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1328259700937')]/cmd:Components/cmd:Soundbites/cmd:Collection/cmd:GeneralInfo/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1328259700937')]/cmd:Components/cmd:Soundbites/cmd:Collection/cmd:GeneralInfo/cmd:Title/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1331113992512')]/cmd:Components/cmd:SL-IPROSLA/cmd:SL-Session/cmd:ResourceShortName/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1345561703620')]/cmd:Components/cmd:collection/cmd:CollectionInfo/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1345561703620')]/cmd:Components/cmd:collection/cmd:CollectionInfo/cmd:Title/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1361876010653')]/cmd:Components/cmd:DiscAn_TextCorpus/cmd:GeneralInfo/cmd:ResourceName/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1361876010653')]/cmd:Components/cmd:DiscAn_TextCorpus/cmd:GeneralInfo/cmd:ResourceTitle/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1361876010653')]/cmd:Components/cmd:DiscAn_TextCorpus/cmd:Publications/cmd:Publication/cmd:PublicationTitle/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1366895758243')]/cmd:Components/cmd:DiscAn_Case/cmd:TextTitle/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1375880372947')]/cmd:Components/cmd:LESLLA/cmd:Session/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1375880372947')]/cmd:Components/cmd:LESLLA/cmd:Session/cmd:Title/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1375880372947')]/cmd:Components/cmd:LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:MediaFile/cmd:ResourceID/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1375880372947')]/cmd:Components/cmd:LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:ResourceId/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1396012485083')]/cmd:Components/cmd:VALID/cmd:Session/cmd:Name/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1396012485083')]/cmd:Components/cmd:VALID/cmd:Session/cmd:Title/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1396012485083')]/cmd:Components/cmd:VALID/cmd:Session/cmd:Resources/cmd:MediaFile/cmd:ResourceID/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1396012485083')]/cmd:Components/cmd:VALID/cmd:Session/cmd:Resources/cmd:WrittenResource/cmd:ResourceId/text(),
					/cmd:CMD[contains(cmd:Header/cmd:MdProfile,'clarin.eu:cr1:p_1396012485083')]/cmd:Components/cmd:VALID/cmd:Session/cmd:Resources/cmd:TestScores/cmd:ResourceID/text()"/>
				<xsl:if test="exists($title[normalize-space()!=''][1])">
					<dc:title>
						<xsl:value-of select="$title[normalize-space()!=''][1]"/>
					</dc:title>
				</xsl:if>
			</xsl:if>
			<xsl:if test="empty($dc/dc:description[normalize-space()!=''])">
				<xsl:variable name="descr" select="
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1328259700928')])]/cmd:Components/(cmd:Soundbites-recording/cmd:SESSION/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:ActorLanguages/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:ActorLanguages/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Actors/cmd:Actor/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SubjectLanguages/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SubjectLanguages/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SubjectLanguages/cmd:SubjectLanguage/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SubjectLanguages/cmd:SubjectLanguage/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Content/cmd:Modality/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Content/cmd:Modality/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Content/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:Content/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:MediaFile/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:MediaFile/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:WrittenResource/cmd:AnnotationType/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:WrittenResource/cmd:AnnotationType/cmd:Description/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:WrittenResource/cmd:Description/text(),Soundbites-recording/cmd:SESSION/cmd:SessionResources/cmd:WrittenResources/cmd:WrittenResource/cmd:Description/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1328259700937')])]/cmd:Components/(cmd:Soundbites/cmd:Description/cmd:Description/text(),Soundbites/cmd:Collection/cmd:GeneralInfo/cmd:Description/text(),Soundbites/cmd:Collection/cmd:GeneralInfo/cmd:Description/cmd:Description/text(),Soundbites/cmd:Collection/cmd:Project/cmd:Description/text(),Soundbites/cmd:Collection/cmd:Project/cmd:Description/cmd:Description/text(),Soundbites/cmd:Collection/cmd:Creators/cmd:Description/text(),Soundbites/cmd:Collection/cmd:Creators/cmd:Description/cmd:Description/text(),Soundbites/cmd:Collection/cmd:DocumentationLanguages/cmd:Description/text(),Soundbites/cmd:Collection/cmd:DocumentationLanguages/cmd:Description/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1331113992512')])]/cmd:Components/(cmd:SL-IPROSLA/cmd:SL-Session/cmd:Description/text(),SL-IPROSLA/cmd:SL-Session/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:Project/cmd:Description/text(),SL-IPROSLA/cmd:Project/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-Content/cmd:Description/text(),SL-IPROSLA/cmd:SL-Content/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorSigner-ChildLanguage/cmd:ActorLanguages/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorSigner-ChildLanguage/cmd:ActorLanguages/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorSigner-ChildLanguage/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorSigner-ChildLanguage/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:ActorLanguages/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:ActorLanguages/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/text(),SL-IPROSLA/cmd:SL-ActorResearcher/cmd:ActorLanguages/cmd:ActorLanguage/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-MediaFile/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-MediaFile/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-AnnotationDocument/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-AnnotationDocument/cmd:Description/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-SourceVideo/cmd:Description/text(),SL-IPROSLA/cmd:SL-Resources/cmd:SL-SourceVideo/cmd:Description/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1337778924955')])]/cmd:Components/(cmd:lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:language/cmd:language-usage/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:language/cmd:language-usage/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:language/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:language/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:languages/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-affiliation/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-affiliation/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-english/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-english/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-musicality/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-musicality/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-languages/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:lucea-questionnaire-languages/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:lucea-questionnaire/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:physiology/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:physiology/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:audiometry/cmd:audiometryMeasurement/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:audiometry/cmd:audiometryMeasurement/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:audiometry/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:facilitator/cmd:audiometry/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:language/cmd:language-usage/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:language/cmd:language-usage/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:language/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:language/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:languages/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:physiology/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:physiology/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:audiometry/cmd:audiometryMeasurement/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:audiometry/cmd:audiometryMeasurement/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:audiometry/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:audiometry/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-curriculum/cmd:exchange/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-curriculum/cmd:exchange/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-curriculum/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-curriculum/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-english/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-english/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-musicality/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-musicality/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-languages/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:lucea-questionnaire-languages/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:lucea-questionnaire/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:speaker/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:Description/text(),lucea/cmd:lucea-actors/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:MediaFile/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:MediaFile/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Content/cmd:Modality/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Content/cmd:Modality/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Content/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Content/cmd:Description/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Description/text(),lucea/cmd:lucea-recording/cmd:lucea-tasks/cmd:lucea-task/cmd:Description/cmd:Description/text(),lucea/cmd:Description/text(),lucea/cmd:Description/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1345561703620')])]/cmd:Components/(cmd:collection/cmd:CollectionInfo/cmd:Modality/cmd:Description/text(),collection/cmd:CollectionInfo/cmd:Modality/cmd:Description/cmd:Description/text(),collection/cmd:CollectionInfo/cmd:Description/text(),collection/cmd:CollectionInfo/cmd:Description/cmd:Description/text(),collection/cmd:WebReference/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1361876010525')])]/cmd:Components/(cmd:DiscAn_Project/cmd:Project/cmd:Institution/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Project/cmd:Institution/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Project/cmd:Cooperations/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Project/cmd:Cooperations/cmd:Cooperation/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Project/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Project/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Access/cmd:DeploymentToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Access/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Access/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:SegmentationUnits/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:TagsetInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:AnnotationTypes/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:AnnotationToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Annotation/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Documentations/cmd:Documentation/cmd:DocumentationLanguages/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Documentations/cmd:Documentation/cmd:DocumentationLanguages/cmd:DocumentationLanguage/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Documentations/cmd:Documentation/cmd:Descriptions/cmd:Description/text(),DiscAn_Project/cmd:Documentations/cmd:Descriptions/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1361876010653')])]/cmd:Components/(cmd:DiscAn_TextCorpus/cmd:GeneralInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Institution/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Institution/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Cooperations/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Cooperations/cmd:Cooperation/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Project/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Publications/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Publications/cmd:Publication/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Documentations/cmd:Documentation/cmd:DocumentationLanguages/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Documentations/cmd:Documentation/cmd:DocumentationLanguages/cmd:DocumentationLanguage/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Documentations/cmd:Documentation/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Documentations/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:CorpusContext/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Creators/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Creators/cmd:Creator/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:CreationToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:SegmentationUnits/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:TagsetInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:AnnotationTypes/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:AnnotationToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Annotation/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:MediaFiles/cmd:MediaFile/cmd:Access/cmd:DeploymentToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:MediaFiles/cmd:MediaFile/cmd:Access/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:MediaFiles/cmd:MediaFile/cmd:Access/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:MediaFiles/cmd:MediaFile/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:MediaFiles/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:Derivation/cmd:DerivationToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:Derivation/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Source/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Creation/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Access/cmd:DeploymentToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Access/cmd:Contact/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:Access/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:SubjectLanguages/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:SubjectLanguages/cmd:SubjectLanguage/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:DocumentationLanguages/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:DocumentationLanguages/cmd:DocumentationLanguage/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:SizeInfo/cmd:TotalSize/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:SizeInfo/cmd:SizePerLanguage/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:ModalityInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:ValidationGrp/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:TextTechnical/cmd:Descriptions/cmd:Description/text(),DiscAn_TextCorpus/cmd:TextTechnical/cmd:LanguageScripts/cmd:Descriptions/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1366895758243')])]/cmd:Components/(cmd:DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:SegmentationUnits/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:TagsetInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:AnnotationTypes/cmd:AnnotationType/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:AnnotationTypes/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:AnnotationToolInfo/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:Annotationtypes-DiscAn/cmd:Annotation/cmd:Descriptions/cmd:Description/text(),DiscAn_Case/cmd:ModalityInfo/cmd:Descriptions/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1375880372947')])]/cmd:Components/(cmd:LESLLA/cmd:Description/text(),LESLLA/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Description/text(),LESLLA/cmd:Session/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Project/cmd:Description/text(),LESLLA/cmd:Session/cmd:Project/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Languages/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Languages/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Languages/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Languages/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Description/text(),LESLLA/cmd:Session/cmd:Content/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Language/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Language/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:Description/text(),LESLLA/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:MediaFile/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:MediaFile/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:MediaFile/cmd:Access/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:MediaFile/cmd:Access/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:Validation/cmd:descriptions/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:Access/cmd:Description/text(),LESLLA/cmd:Session/cmd:Utterances/cmd:Utterance/cmd:Resources/cmd:WrittenResource/cmd:Access/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:Anonyms/cmd:Access/cmd:Description/text(),LESLLA/cmd:Session/cmd:Anonyms/cmd:Access/cmd:Description/cmd:Description/text(),LESLLA/cmd:Session/cmd:References/cmd:Description/text(),LESLLA/cmd:Session/cmd:References/cmd:Description/cmd:Description/text())),';'),
					string-join(distinct-values(/cmd:CMD[exists(cmd:Header/cmd:MdProfile[contains(.,'clarin.eu:cr1:p_1396012485083')])]/cmd:Components/(cmd:VALID/cmd:Description/text(),VALID/cmd:Session/cmd:Description/text(),VALID/cmd:Session/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Project/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Languages/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Languages/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Languages/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Languages/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Description/text(),VALID/cmd:Session/cmd:Content/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:OtherDetails/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:ActorCharacteristics/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:MotherTongue/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:PrimaryLanguage/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:SecondaryLanguage/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Language/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Language/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:HomeLanguage2/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Languages/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Description/text(),VALID/cmd:Session/cmd:Actors/cmd:Actor/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:MediaFile/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:MediaFile/cmd:Access/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:WrittenResource/cmd:Validation/cmd:descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:WrittenResource/cmd:Access/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:WrittenResource/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:TestScores/cmd:Description/text(),VALID/cmd:Session/cmd:Resources/cmd:TestScores/cmd:Access/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Session/cmd:Anonyms/cmd:Access/cmd:Description/text(),VALID/cmd:Session/cmd:Anonyms/cmd:Access/cmd:Description/cmd:Description/text(),VALID/cmd:Session/cmd:References/cmd:Description/text(),VALID/cmd:Session/cmd:References/cmd:Description/cmd:Description/text(),VALID/cmd:Documentation/cmd:DocumentationLanguages/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Documentation/cmd:DocumentationLanguages/cmd:DocumentationLanguage/cmd:Descriptions/cmd:Description/text(),VALID/cmd:Documentation/cmd:Descriptions/cmd:Description/text(),VALID/cmd:References/cmd:Descriptions/cmd:Description/text())),';')"/>
				<xsl:if test="exists($descr[normalize-space()!=''][1])">
					<dc:description>
						<xsl:value-of select="$descr[normalize-space()!=''][1]"/>
					</dc:description>
				</xsl:if>
			</xsl:if>
			<xsl:copy-of select="$dc"/>
			<xsl:comment>extra dc:description to enable full text search of the CMD record</xsl:comment>
			<dc:description>
				<xsl:value-of select="string-join((cmd:Header,cmd:Components)/tokenize(.,'\s+')[normalize-space(.)!=''],' ')"/>
			</dc:description>
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
