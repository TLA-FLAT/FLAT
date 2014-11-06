<?xml version="1.0" encoding="UTF-8"?>
<p:declare-step xmlns:p="http://www.w3.org/ns/xproc" version="1.0"
    xmlns:c="http://www.w3.org/ns/xproc-step"
    xmlns:l="http://xproc.org/library"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:cmd="http://www.clarin.eu/cmd/"
    xmlns:xs="http://www.w3.org/2001/XMLSchema"
    xmlns:ann="http://www.clarin.eu">
	<!-- some external libraries -->
	<p:import href="http://xproc.org/library/recursive-directory-list.xpl"/>
	<p:import href="http://xproc.org/library/xml-schema-report.xpl"/>
	<!-- no output -->
    <p:output port="result" primary="true" sequence="true">
    	<p:empty/>
    </p:output>
	<!-- the dir with IMDI files and resources to import into Fedora/Islandora -->
	<p:variable name="dir" select="'file:///Users/menzowindhouwer/Documents/Projects/EasyLAT/test/'"/>
	<!-- find all the files in the dir and its subdirs -->
    <l:recursive-directory-list>
        <p:with-option name="path" select="$dir"/>
    </l:recursive-directory-list>
	<!-- make sure the files have a base URI -->
    <p:add-xml-base all="true" relative="false"/>
	<!-- convert the IMDI files -->
    <p:for-each name="convert-imdi">
        <p:output port="result" primary="true" sequence="true">
            <p:empty/>
        </p:output>
    	<p:output port="imdi-validation-reports" primary="false" sequence="true">
    		<p:pipe step="validate-imdi-report" port="result"/>
    	</p:output>
    	<p:output port="cmd-validation-reports" primary="false" sequence="true">
    		<p:pipe step="validate-cmd-report" port="result"/>
    	</p:output>
    	<p:log port="imdi-validation-reports" href="./imdi-validation-reports.xml"/>
    	<p:log port="cmd-validation-reports" href="./cmd-validation-reports.xml"/>
    	<!-- select only IMDI files, skip hidden files and the corpman and sessions directories -->
        <p:iteration-source select="//c:file[ends-with(@name,'.imdi')][not(starts-with(@name,'.'))][empty(ancestor::c:directory[@name=('corpman','sessions')])]"/>
    	<!-- a IMDI file -->
        <p:variable name="imdi" select="p:resolve-uri(/*/@name,/*/@xml:base)"/>
    	<!-- to be created CMD file -->
        <p:variable name="cmdi" select="replace($imdi,'.imdi','.cmdi')"/>
    	<!-- to be created DC file -->
    	<p:variable name="dc" select="replace($imdi,'.imdi','.dc')"/>
    	<!-- load the IMDI file -->
    	<p:load name="load-imdi">
            <p:with-option name="href" select="$imdi"/>
        </p:load>
    	<!-- validate the IMDI file -->
    	<l:xml-schema-report name="validate-imdi">
    		<p:input port="source">
    			<p:pipe port="result" step="load-imdi"/>
    		</p:input>
    		<p:input port="schema">
    			<p:document href="./includes/IMDI_3.0.xsd"/>
    		</p:input>
    	</l:xml-schema-report>
    	<!-- report the IMDI validation results -->
    	<p:for-each name="validate-imdi-report">
    		<p:iteration-source>
    			<p:pipe port="report" step="validate-imdi"/>
    		</p:iteration-source>
    		<p:output port="result" sequence="true">
    			<p:pipe port="result" step="localized-imdi-errors"/>
    		</p:output>
    		<p:add-attribute match="/c:errors" name="localized-imdi-errors">
    			<p:with-option name="attribute-name" select="'file'"/>
    			<p:with-option name="attribute-value" select="$imdi"/>
    		</p:add-attribute>
    	</p:for-each>
    	<!-- convert the IMDI file to CMDI -->
    	<p:xslt name="imdi2cmdi">
    		<p:input port="stylesheet">
    			<p:document href="./includes/imdi2cmdi.xslt"/>
    		</p:input>
    		<p:input port="source">
    			<p:pipe port="result" step="load-imdi"></p:pipe>
    		</p:input>
    		<p:with-param name="uri-base" select="$cmdi"/>
        </p:xslt>
    	<!-- to validate the resulting CMD file get the corresponding CMD schema -->
        <p:try name="cmd-schema">
            <p:group>
                <p:output port="result">
                    <p:pipe port="result" step="load-cmd-schema"/>
                </p:output>
            	<!-- load the CMD schema from the profile cache directory -->
            	<p:load name="load-cmd-schema">
                    <p:with-option name="href" select="concat('./profile-cache/',replace(/cmd:CMD/@xsi:schemaLocation,'.*(clarin.eu:cr1:p_[0-9]+).*','$1'),'.xsd')"/>
                </p:load>
            </p:group>
            <p:catch>
                <p:output port="result">
                    <p:pipe port="result" step="open-cmd-schema"/>
                </p:output>
            	<!-- if this fails, load the CMD schema from the component registry -->
            	<p:load>
                    <p:with-option name="href" select="replace(/cmd:CMD/@xsi:schemaLocation,'.*(http://catalog.clarin.eu/ds/ComponentRegistry/rest/registry/profiles/clarin.eu:cr1:p_[0-9]+/xsd).*','$1')"/>
                </p:load>
            	<!-- make the CMD envelop open for foreign attributes -->
            	<!-- CHECK: when CMD allows foreign attributes this transform can be deleted -->
            	<p:xslt name="open-cmd-schema">
            		<p:input port="parameters">
            			<p:empty/>
            		</p:input>
            		<p:input port="stylesheet">
            			<p:document href="./includes/makeCmdiXsdOpen.xsl"/>
            		</p:input>
            	</p:xslt>
            	<!-- store the CMD schema in the profile cache directory -->
                <p:store>
                    <p:input port="source">
                        <p:pipe port="result" step="open-cmd-schema"/>
                    </p:input>
                    <p:with-option name="href" select="concat('./profile-cache/',/xs:schema/xs:annotation/xs:appinfo/ann:Header/ann:ID,'.xsd')"/>
                </p:store>
            </p:catch>
        </p:try>
    	<!-- validate the CMD file agains the CMD schema -->
        <l:xml-schema-report name="cmd-validate">
            <p:input port="source">
            	<p:pipe port="result" step="imdi2cmdi"/>
            </p:input>
            <p:input port="schema">
                <p:pipe port="result" step="cmd-schema"/>
            </p:input>
        </l:xml-schema-report>
    	<!-- report the CMD validation results -->
        <p:for-each name="validate-cmd-report">
            <p:iteration-source>
                <p:pipe port="report" step="cmd-validate"/>
            </p:iteration-source>
            <p:output port="result" sequence="true">
            	<p:pipe port="result" step="localized-cmd-errors"/>
            </p:output>
        	<p:add-attribute match="/c:errors" name="localized-cmd-errors">
                <p:with-option name="attribute-name" select="'file'"/>
                <p:with-option name="attribute-value" select="$cmdi"/>
            </p:add-attribute>
        </p:for-each>
    	<!-- store the CMDI file -->
    	<p:store>
    		<p:input port="source">
    			<p:pipe port="result" step="imdi2cmdi"/>
    		</p:input>
    		<p:with-option name="href" select="$cmdi"/>
    	</p:store>
    	<!-- also transform the IMDI file to Dublin Core -->
    	<p:xslt>
    		<p:input port="source">
    			<p:pipe port="result" step="load-imdi"/>
    		</p:input>
    		<p:input port="parameters">
    			<p:empty/>
    		</p:input>
    		<p:input port="stylesheet">
    			<p:document href="./includes/imdi_3_0_olac_dc.xsl"/>
    		</p:input>
    	</p:xslt>
    	<!-- store the DC file -->
    	<p:store>
    		<p:with-option name="href" select="$dc"/>
    	</p:store>
    </p:for-each>
	<!-- create a lookup file for all relations specified in the CMDI files -->
	<!-- NOTE: the XSLT contains a collection call that loads all CMDI files -->
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
	<!-- find all the files in the dir and its subdirs -->
	<l:recursive-directory-list>
		<p:with-option name="path" select="$dir"/>
	</l:recursive-directory-list>
	<!-- make sure the files have a base URI -->
	<p:add-xml-base all="true" relative="false"/>
	<!-- create the FOX files -->
	<p:for-each name="create-foxes">
		<p:output port="fox-validation-reports" primary="false" sequence="true">
			<p:pipe step="validate-fox-report" port="result"/>
		</p:output>
		<p:output port="resource-fox-validation-reports" primary="false" sequence="true">
			<p:pipe step="resource-foxes" port="validate-resource-fox-report"/>
		</p:output>
		<p:log port="fox-validation-reports" href="./fox-validation-reports.xml"/>
		<p:log port="resource-fox-validation-reports" href="./resource-fox-validation-reports.xml"/>
		<!-- select only CMD files, skip hidden files and the corpman and sessions directories -->
		<p:iteration-source select="//c:file[ends-with(@name,'.cmdi')][not(starts-with(@name,'.'))][empty(ancestor::c:directory[@name=('corpman','sessions')])]"/>
		<!-- a CMD file -->
		<p:variable name="cmdi" select="p:resolve-uri(/*/@name,/*/@xml:base)"/>
		<!-- to be created FOX file -->
		<p:variable name="fox" select="replace($cmdi,'.cmdi','.fox')"/>
		<!-- load the CMD file -->
		<p:load name="load">
			<p:with-option name="href" select="$cmdi"/>
		</p:load>
		<!-- convert the CMD file to FOXML -->
		<p:xslt name="cmd2fox">
			<p:with-param name="rels-uri" select="p:resolve-uri('./relations.xml',$dir)"/>
			<p:input port="stylesheet">
				<p:document href="./cmd2fox.xsl"/>
			</p:input>
		</p:xslt>
		<!-- validate the FOX file -->
		<l:xml-schema-report name="validate-fox">
			<p:input port="source">
				<p:pipe port="result" step="cmd2fox"/>
			</p:input>
			<p:input port="schema">
				<p:document href="./includes/foxml1-1.xsd"/>
			</p:input>
		</l:xml-schema-report>
		<!-- report the FOX validation results -->
		<p:for-each name="validate-fox-report">
			<p:iteration-source>
				<p:pipe port="report" step="validate-fox"/>
			</p:iteration-source>
			<p:output port="result" sequence="true">
				<p:pipe port="result" step="localized-fox-errors"/>
			</p:output>
			<p:add-attribute match="/c:errors" name="localized-fox-errors">
				<p:with-option name="attribute-name" select="'file'"/>
				<p:with-option name="attribute-value" select="$fox"/>
			</p:add-attribute>
		</p:for-each>
		<!-- store the FOX file -->
		<p:store>
			<p:input port="source">
				<p:pipe port="result" step="cmd2fox"/>
			</p:input>
			<p:with-option name="href" select="$fox"/>
		</p:store>
		<!-- all the FOX files related to resources are on the secondary output, store these as well -->
		<p:for-each name="resource-foxes">
			<p:output port="validate-resource-fox-report">
				<p:pipe port="result" step="validate-resource-fox-report"></p:pipe>
			</p:output>
			<p:iteration-source>
				<p:pipe port="secondary" step="cmd2fox"/>
			</p:iteration-source>
			<p:variable name="path" select="base-uri(/*)"/>
			<!-- validate the FOX file -->
			<l:xml-schema-report name="validate-resource-fox">
				<p:input port="schema">
					<p:document href="./includes/foxml1-1.xsd"/>
				</p:input>
			</l:xml-schema-report>
			<!-- report the FOX validation results -->
			<p:for-each name="validate-resource-fox-report">
				<p:iteration-source>
					<p:pipe port="report" step="validate-resource-fox"/>
				</p:iteration-source>
				<p:output port="result" sequence="true">
					<p:pipe port="result" step="localized-resource-fox-errors"/>
				</p:output>
				<p:add-attribute match="/c:errors" name="localized-resource-fox-errors">
					<p:with-option name="attribute-name" select="'file'"/>
					<p:with-option name="attribute-value" select="$path"/>
				</p:add-attribute>
			</p:for-each>
			<p:store>
				<p:with-option name="href" select="$path"/>
			</p:store>
		</p:for-each>
	</p:for-each>
</p:declare-step>