<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	version="2.0"
	xmlns:xs="http://www.w3.org/2001/XMLSchema"
	xmlns:foxml="info:fedora/fedora-system:def/foxml#"
    xmlns:cmd="http://www.clarin.eu/cmd/"
    xmlns:lat="http://lat.mpi.nl/"
    xmlns:iso="http://www.iso.org/"
    xmlns:sil="http://www.sil.org/">
	
	<xsl:variable name="sil-lang-top" select="document('sil_to_iso6393.xml')/sil:languages"/>
	<xsl:key name="sil-lookup" match="sil:lang" use="sil:sil"/>
	
	<xsl:variable name="iso-lang-uri" select="'iso2iso.xml'"/>
	<xsl:variable name="iso-lang-doc" select="document($iso-lang-uri)"/>
	<xsl:variable name="iso-lang-top" select="$iso-lang-doc/iso:m"/>
	<xsl:key name="iso639_1-lookup" match="iso:e" use="iso:o"/>
	<xsl:key name="iso639_2-lookup" match="iso:e" use="iso:b|iso:t"/>
	<xsl:key name="iso639_3-lookup" match="iso:e" use="iso:i"/>
	<xsl:key name="iso639-lookup" match="iso:e" use="iso:i|iso:o|iso:b|iso:t"/>
	
	<xsl:function name="lat:lang2iso">
		<xsl:param name="language"/>
		<xsl:variable name="codeset" select="replace(substring-before($language,':'),' ','')"/>
		<xsl:variable name="codestr" select="substring-after($language,':')"/>
		<xsl:variable name="code">
			<xsl:choose>
				<xsl:when test="$codeset='ISO639-1'">
					<xsl:choose>
						<xsl:when test="$codestr='xxx'">
							<xsl:value-of select="'und'"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:variable name="iso" select="key('iso639_1-lookup', $codestr, $iso-lang-top)/iso:i"/>
							<xsl:choose>
								<xsl:when test="$iso!='xxx'">
									<xsl:value-of select="$iso"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is not a ISO 639-1 language code, falling back to und.</xsl:message>
									<xsl:value-of select="'und'"/>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="$codeset='ISO639-2'">
					<xsl:choose>
						<xsl:when test="$codestr='xxx'">
							<xsl:value-of select="'und'"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:variable name="iso" select="key('iso639_2-lookup', $codestr, $iso-lang-top)/iso:i"/>
							<xsl:choose>
								<xsl:when test="$iso!='xxx'">
									<xsl:value-of select="$iso"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is not a ISO 639-2 language code, falling back to und.</xsl:message>
									<xsl:value-of select="'und'"/>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="$codeset='ISO639-3'">
					<xsl:choose>
						<xsl:when test="$codestr='xxx'">
							<xsl:value-of select="'und'"/>
						</xsl:when>
						<xsl:when test="exists(key('iso639_3-lookup', $codestr, $iso-lang-top))">
							<xsl:value-of select="$codestr"/>
						</xsl:when>
						<xsl:otherwise>
							<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is not a ISO 639-3 language code, falling back to und.</xsl:message>
							<xsl:value-of select="'und'"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="$codeset='ISO639'">
					<xsl:choose>
						<xsl:when test="$codestr='xxx'">
							<xsl:value-of select="'und'"/>
						</xsl:when>
						<xsl:when test="exists(key('iso639-lookup', $codestr, $iso-lang-top))">
							<xsl:variable name="iso" select="key('iso639-lookup', $codestr, $iso-lang-top)/iso:i"/>
							<xsl:choose>
								<xsl:when test="$iso!='xxx'">
									<xsl:value-of select="$iso"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is not a ISO 639 language code, falling back to und.</xsl:message>
									<xsl:value-of select="'und'"/>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:when>
						<xsl:otherwise>
							<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is not a ISO 639 language code, falling back to und.</xsl:message>
							<xsl:value-of select="'und'"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="$codeset='RFC1766'">
					<xsl:choose>
						<xsl:when test="starts-with($codestr,'x-sil-')">
							<xsl:variable name="iso" select="key('sil-lookup', lower-case(replace($codestr, 'x-sil-', '')), $sil-lang-top)/sil:iso"/>
							<xsl:choose>
								<xsl:when test="$iso!='xxx'">
									<xsl:value-of select="$iso"/>
								</xsl:when>
								<xsl:otherwise>
									<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] is SIL code (?) with an unknown mapping to ISO 639, falling back to und.</xsl:message>
									<xsl:value-of select="'und'"/>
								</xsl:otherwise>
							</xsl:choose>
						</xsl:when>
						<xsl:otherwise>
							<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] has no known mapping to ISO 639, falling back to und.</xsl:message>
							<xsl:value-of select="'und'"/>
						</xsl:otherwise>
					</xsl:choose>
				</xsl:when>
				<xsl:otherwise>
					<xsl:message>WRN: [<xsl:value-of select="$codestr"/>] has no known mapping to ISO 639, falling back to und.</xsl:message>
					<xsl:value-of select="'und'"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>
		<xsl:sequence select="$code"/>
	</xsl:function>
    
    <xsl:template match="cmd:CMD" mode="other">
    	<xsl:param name="pid" tunnel="yes"/>
    	<xsl:param name="base" tunnel="yes"/>
    	<xsl:if test="cmd:Header/cmd:MdProfile=('clarin.eu:cr1:p_1407745712035','clarin.eu:cr1:p_1417617523856')">
    		<foxml:datastream xmlns:foxml="info:fedora/fedora-system:def/foxml#" ID="OLAC" STATE="A" CONTROL_GROUP="X">
    			<foxml:datastreamVersion ID="OLAC.0" FORMAT_URI="cf" LABEL="OLAC Record for this object" MIMETYPE="application/xml">
    				<foxml:xmlContent>
    					<olac:olac xmlns:olac="http://www.language-archives.org/OLAC/1.1/" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.language-archives.org/OLAC/1.1/ http://www.language-archives.org/OLAC/1.1/olac.xsd">
    						<dc:identifier xsi:type="dcterms:URI">
    							<xsl:value-of select="$pid"/>
    						</dc:identifier>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Name" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Title" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:*[not(local-name() = ('Title','Name'))]" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:MDGroup/cmd:Location/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Project/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Project/cmd:Contact/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Content/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Content/cmd:Languages/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Actors/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:Resources/cmd:Source/cmd:*" mode="olac"/>
    						<xsl:apply-templates select="cmd:Components/*/cmd:References/cmd:*" mode="olac"/>
    					</olac:olac>
    				</foxml:xmlContent>
    			</foxml:datastreamVersion>
    		</foxml:datastream>
    	</xsl:if>
    </xsl:template>
	
	<xsl:template match="text()" mode="other"/>
	<xsl:template match="text()" mode="olac"/>
	
	<!-- Session|Corpus.Name -->
	<xsl:template match="cmd:Components/*/cmd:Name" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session|Corpus.Title -->
	<xsl:template match="cmd:Components/*/cmd:Title" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Date -->
	<xsl:template match="cmd:Components/*/cmd:Date" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:date</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Location.Country -->
	<xsl:template match="cmd:Components/*/cmd:Location/cmd:Country" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:coverage</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Title -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Title" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:title</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Id -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Id" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:identifier</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Contact.Name -->
	<xsl:template match="cmd:Components/*/cmd:Contact/cmd:Name" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:publisher</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Project.Contact.Organisation -->
	<xsl:template match="cmd:Components/*/cmd:Project/cmd:Contact/cmd:Organisation" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:publisher</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.Genre -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:Genre" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.SubGenre -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:SubGenre" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.Languages.Language.* -->
	<xsl:template match="cmd:Components/*/cmd:Content/cmd:Content_Languages/cmd:Content_Language"
		mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="olac-type">olac:language</xsl:with-param>
			<xsl:with-param name="olac-code" select="lat:lang2iso(cmd:Id)"/>
			<xsl:with-param name="value-node" select="cmd:Name"/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Actors.Actor -->
	<xsl:template match="cmd:Components/*/cmd:Actors/cmd:Actor" mode="olac">
		<xsl:apply-templates select="cmd:Name" mode="#current"/>
		<xsl:apply-templates select="cmd:Description" mode="#current"/>
	</xsl:template>
	<!-- Session.Actors.Actor.Role -->
	<xsl:template match="cmd:Components/*/cmd:Actors/cmd:Actor/cmd:Name" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:contributor</xsl:with-param>
			<xsl:with-param name="olac-type">olac:role</xsl:with-param>
			<xsl:with-param name="olac-code">
				<xsl:choose>
					<xsl:when test="../cmd:Role = 'Annotator'">annotator</xsl:when>
					<xsl:when test="../cmd:Role = 'Author'">author</xsl:when>
					<xsl:when test="../cmd:Role = 'Consultant'">consultant</xsl:when>
					<xsl:when test="../cmd:Role = 'Depositor'">depositor</xsl:when>
					<xsl:when test="../cmd:Role = 'Editor'">editor</xsl:when>
					<xsl:when test="../cmd:Role = 'Illustrator'">illustrator</xsl:when>
					<xsl:when test="../cmd:Role = 'Interviewer'">interviewer</xsl:when>
					<xsl:when test="../cmd:Role = 'Photographer'">photographer</xsl:when>
					<xsl:when test="../cmd:Role = 'Recorder'">recorder</xsl:when>
					<xsl:when test="../cmd:Role = 'Researcher'">researcher</xsl:when>
					<xsl:when test="../cmd:Role = 'Singer'">singer</xsl:when>
					<xsl:when test="../cmd:Role = 'Speaker/Signer'">speaker</xsl:when>
					<xsl:when test="../cmd:Role = 'Translator'">translator</xsl:when>
					<xsl:otherwise/>
				</xsl:choose>
			</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.MediaFile.Type -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:Type" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:type</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.MediaFile.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:MediaFile/cmd:Format" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:Format" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.ContentEncoding -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:ContentEncoding"
		mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.WrittenResource.CharacterEncoding -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:WrittenResource/cmd:CharacterEncoding"
		mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Resources.Source.Format -->
	<xsl:template match="cmd:Components/*/cmd:Resources/cmd:Source/cmd:Format" mode="olac">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:format</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	<!-- Session.Content.Subject -->
	<xsl:template match="cmd:Components/*/cmd:MDGroup/cmd:Content/cmd:Subject">
		<xsl:call-template name="create-olac-element">
			<xsl:with-param name="dc-name">dc:subject</xsl:with-param>
			<xsl:with-param name="olac-type">olac:linguistic-field</xsl:with-param>
			<xsl:with-param name="value-node" select="."/>
		</xsl:call-template>
	</xsl:template>
	
	
	<!-- Description (sub) -->
	<xsl:template match="cmd:Description" mode="olac">
		<xsl:if test="not(normalize-space(.) = '')">
			<dc:description xmlns:dc="http://purl.org/dc/elements/1.1/">
				<xsl:copy-of select="@xml:lang"/>
				<xsl:value-of select="text()"/>
				<xsl:if test="not(@Link = '')">
					<xsl:value-of select="@Link"/>
				</xsl:if>
			</dc:description>
		</xsl:if>
	</xsl:template>
	
	<!-- create dublin core element -->
	<xsl:template name="create-olac-element">
		<xsl:param name="dc-name"/>
		<xsl:param name="olac-type" select="()"/>
		<xsl:param name="olac-code" select="()"/>
		<xsl:param name="value-node" select="()"/>
		<xsl:if
			test="not(normalize-space($value-node) = '') and (normalize-space($value-node) != 'Unspecified')">
			<xsl:element name="{$dc-name}" xmlns:dc="http://purl.org/dc/elements/1.1/"
				xmlns:dcterms="http://purl.org/dc/terms/" xmlns:olac="http://www.language-archives.org/OLAC/1.1/"
				xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
				<xsl:choose>
					<xsl:when test="normalize-space($olac-type)!=''">
						<xsl:attribute name="xsi:type" select="$olac-type"/>
						<xsl:choose>
							<xsl:when test="normalize-space($olac-code)!=''">
								<xsl:attribute name="olac:code" select="normalize-space($olac-code)"/>
								<xsl:if test="normalize-space($value-node)!=''">
									<xsl:value-of select="normalize-space($value-node)"/>
								</xsl:if>
							</xsl:when>
							<xsl:otherwise>
								<xsl:attribute name="olac:code" select="normalize-space($value-node)"/>
							</xsl:otherwise>
						</xsl:choose>
					</xsl:when>
					<xsl:otherwise>
						<xsl:value-of select="normalize-space($value-node)"/>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:element>
		</xsl:if>
	</xsl:template>

</xsl:stylesheet>