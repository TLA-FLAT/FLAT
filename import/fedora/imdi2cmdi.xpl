<?xml version="1.0" encoding="UTF-8"?>
<p:declare-step xmlns:p="http://www.w3.org/ns/xproc" version="1.0"
    xmlns:c="http://www.w3.org/ns/xproc-step"
    xmlns:l="http://xproc.org/library"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:cmd="http://www.clarin.eu/cmd/"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:ann="http://www.clarin.eu">
    <p:output port="result" primary="true" sequence="true">
        <!--<p:pipe port="result" step="reports"/>-->
    	<p:empty/>
    </p:output>
	<p:variable name="dir" select="'file:///Users/menzowindhouwer/Documents/Projects/EasyLAT/test/'"/>
    <p:import href="http://xproc.org/library/recursive-directory-list.xpl"/>
    <p:import href="http://xproc.org/library/xml-schema-report.xpl"/>
    <l:recursive-directory-list>
        <p:with-option name="path" select="$dir"/>
    </l:recursive-directory-list>
    <p:add-xml-base all="true" relative="false"/>
    <p:for-each name="reports">
        <p:output port="result" primary="true" sequence="true">
            <p:pipe port="result" step="report"/>
        </p:output>
        <p:iteration-source select="//c:file[ends-with(@name,'.imdi')][not(starts-with(@name,'.'))][empty(ancestor::c:directory[@name=('corpman','sessions')])]"/>
        <p:variable name="imdi" select="p:resolve-uri(/*/@name,/*/@xml:base)"/>
        <p:variable name="cmdi" select="replace($imdi,'.imdi','.cmdi')"/>
    	<p:variable name="dc" select="replace($imdi,'.imdi','.dc')"/>
    	<p:load name="load">
            <p:with-option name="href" select="$imdi"/>
        </p:load>
        <p:xslt name="transform">
            <p:with-param name="uri-base" select="$cmdi"/>
            <p:input port="stylesheet">
                <p:document href="./includes/imdi2cmdi.xslt"/>
            </p:input>
        </p:xslt>
        <p:try name="schema">
            <p:group>
                <p:output port="result">
                    <p:pipe port="result" step="load-schema"/>
                </p:output>
                <p:load name="load-schema">
                    <p:with-option name="href" select="concat('./profile-cache/',replace(/cmd:CMD/@xsi:schemaLocation,'.*(clarin.eu:cr1:p_[0-9]+).*','$1'),'.xsd')"/>
                </p:load>
            </p:group>
            <p:catch>
                <p:output port="result">
                    <p:pipe port="result" step="open-schema"/>
                </p:output>
                <p:load>
                    <p:with-option name="href" select="replace(/cmd:CMD/@xsi:schemaLocation,'.*(http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_[0-9]+/xsd).*','$1')"/>
                </p:load>
            	<p:xslt name="open-schema">
            		<p:input port="parameters">
            			<p:empty/>
            		</p:input>
            		<p:input port="stylesheet">
            			<p:document href="./includes/makeCmdiXsdOpen.xsl"/>
            		</p:input>
            	</p:xslt>
                <p:store>
                    <p:input port="source">
                        <p:pipe port="result" step="open-schema"/>
                    </p:input>
                    <p:with-option name="href" select="concat('./profile-cache/',/xs:schema/xs:annotation/xs:appinfo/ann:Header/ann:ID,'.xsd')"/>
                </p:store>
            </p:catch>
        </p:try>
        <l:xml-schema-report name="validate">
            <p:input port="source">
                <p:pipe port="result" step="transform"/>
            </p:input>
            <p:input port="schema">
                <p:pipe port="result" step="schema"/>
            </p:input>
        </l:xml-schema-report>
        <p:for-each name="report">
            <p:iteration-source>
                <p:pipe port="report" step="validate"/>
            </p:iteration-source>
            <p:output port="result" sequence="true">
                <p:pipe port="result" step="extended-errors"/>
            </p:output>
            <p:add-attribute match="/c:errors" name="extended-errors">
                <p:with-option name="attribute-name" select="'file'"/>
                <p:with-option name="attribute-value" select="$cmdi"/>
            </p:add-attribute>
        </p:for-each>
        <p:store>
            <p:input port="source">
                <p:pipe port="result" step="validate"/>
            </p:input>
            <p:with-option name="href" select="$cmdi"/>
        </p:store>
    	<!-- also generate Dublin Core -->
    	<p:xslt>
    		<p:input port="source">
    			<p:pipe port="result" step="load"/>
    		</p:input>
    		<p:input port="parameters">
    			<p:empty/>
    		</p:input>
    		<p:input port="stylesheet">
    			<p:document href="./includes/imdi_3_0_olac_dc.xsl"/>
    		</p:input>
    	</p:xslt>
    	<p:store>
    		<p:with-option name="href" select="$dc"/>
    	</p:store>
    </p:for-each>
	<p:xslt>
		<p:input port="source">
			<p:inline>
				<null/>
			</p:inline>
		</p:input>
		<p:with-param name="dir" select="$dir"/>
		<p:input port="stylesheet">
			<p:document href="./cmd2rels.xsl"/>
		</p:input>
	</p:xslt>
	<p:store>
		<p:with-option name="href" select="p:resolve-uri('./relations.xml',$dir)"/>
	</p:store>
	<l:recursive-directory-list>
		<p:with-option name="path" select="$dir"/>
	</l:recursive-directory-list>
	<p:add-xml-base all="true" relative="false"/>
	<p:for-each name="foxes">
		<p:iteration-source select="//c:file[ends-with(@name,'.cmdi')][not(starts-with(@name,'.'))][empty(ancestor::c:directory[@name=('corpman','sessions')])]"/>
		<p:variable name="cmdi" select="p:resolve-uri(/*/@name,/*/@xml:base)"/>
		<p:variable name="fox" select="replace($cmdi,'.cmdi','.fox')"/>
		<p:load name="load">
			<p:with-option name="href" select="$cmdi"/>
		</p:load>
		<!-- generate FOXML -->
		<p:xslt name="fox">
			<p:with-param name="rels-uri" select="p:resolve-uri('./relations.xml',$dir)"/>
			<p:input port="stylesheet">
				<p:document href="./cmd2fox.xsl"/>
			</p:input>
		</p:xslt>
		<p:store>
			<p:with-option name="href" select="$fox"/>
		</p:store>
		<p:for-each>
			<p:iteration-source>
				<p:pipe port="secondary" step="fox"/>
			</p:iteration-source>
			<p:variable name="path" select="base-uri(/*)"/>
			<p:store>
				<p:with-option name="href" select="$path"/>
			</p:store>
		</p:for-each>
	</p:for-each>
</p:declare-step>