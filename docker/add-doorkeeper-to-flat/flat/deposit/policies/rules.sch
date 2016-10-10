<schema xmlns="http://purl.oclc.org/dsdl/schematron" queryBinding="xslt2">
    <ns uri="http://www.clarin.eu/cmd/" prefix="cmd11"/>
    <ns uri="http://www.clarin.eu/cmd/1" prefix="cmd12"/>
    <ns uri="http://www.w3.org/2001/XMLSchema-instance" prefix="xsi"/>

    <pattern>
        <rule role="warning" context="cmd11:Header">
            <assert test="string-length(cmd11:MdProfile/text()) &gt; 0">
                [CMDI Best Practices] A CMDI instance should contain a non-empty &lt;cmd:MdProfile&gt; element in &lt;cmd:Header&gt;.
            </assert>
        </rule>   
    </pattern>
    <pattern>
        <rule role="warning" context="cmd12:Header">
            <assert test="string-length(cmd12:MdProfile/text()) &gt; 0">
                [CMDI Best Practices] A CMDI instance should contain a non-empty &lt;cmd:MdProfile&gt; element in &lt;cmd:Header&gt;.
            </assert>
        </rule>   
    </pattern>
    
    <pattern>
        <rule  role="warning" context="cmd11:Header">
            <assert test="string-length(cmd11:MdSelfLink/text()) &gt; 0">
                [CMDI Best Practices] A CMDI instance should contain a non-empty &lt;cmd:MdSelfLink&gt; element in &lt;cmd:Header&gt;.
            </assert>
        </rule>   
    </pattern>
    <pattern>
        <rule  role="warning" context="cmd12:Header">
            <assert test="string-length(cmd12:MdSelfLink/text()) &gt; 0">
                [CMDI Best Practices] A CMDI instance should contain a non-empty &lt;cmd:MdSelfLink&gt; element in &lt;cmd:Header&gt;.
            </assert>
        </rule>   
    </pattern>
    
    <!--
        Rules contributed by Menzo Windhouwer <Menzo.Windhouwer@mpi.nl>
        Reformatted and assert messages slightly reworded
    -->
    <!-- Does the schema reside in the Component Registry? -->
    <pattern>
        <title>Test xsi:schemaLocation</title>
        <rule role="warning" context="/cmd11:CMD">
            <assert test="matches(@xsi:schemaLocation,'http(s)?://catalog.clarin.eu/ds/ComponentRegistry/rest/')">
                [CMDI Best Practice] /cmd:CMD/@xsi:schemaLocation doesn't refer to a schema from the Component Registry! Actual value was [<value-of select="@xsi:schemaLocation"/>]
            </assert>
        </rule>
    </pattern>
    <pattern>
        <title>Test xsi:schemaLocation</title>
        <rule role="warning" context="/cmd12:CMD">
            <assert test="matches(@xsi:schemaLocation,'http(s)?://catalog.clarin.eu/ds/ComponentRegistry/rest/')">
                [CMDI Best Practice] /cmd:CMD/@xsi:schemaLocation doesn't refer to a schema from the Component Registry! [Actual value was [<value-of select="@xsi:schemaLocation"/>]
            </assert>
        </rule>
    </pattern>
    
    <!-- Is there at least one ResourceProxy? -->
    <pattern>
        <title>Test for ResourceProxies</title>
        <rule role="warning" context="/cmd11:CMD/cmd11:Resources/cmd11:ResourceProxyList">
            <assert test="count(cmd11:ResourceProxy) ge 1">
                [CMDI Best Practices] There should be at least one ResourceProxy! Otherwise this is the metadata of what? Itself?
            </assert>
        </rule>
    </pattern>
    <pattern>
        <title>Test for ResourceProxies</title>
        <rule role="warning" context="/cmd12:CMD/cmd12:Resources/cmd12:ResourceProxyList">
            <assert test="count(cmd12:ResourceProxy) ge 1">
                [CMDI Best Practices] There should be at least one ResourceProxy! Otherwise this is the metadata of what? Itself?
            </assert>
        </rule>
    </pattern>
    
    <!-- Can we determine the profile used? -->
    <pattern>
        <title>Test for known profile</title>
        <rule role="warning" context="/cmd11:CMD">
            <assert test="matches(@xsi:schemaLocation,'clarin.eu:cr[0-9]+:p_[0-9]+.+') or matches(cmd11:Header/cmd11:MdProfile,'clarin.eu:cr[0-9]+:p_[0-9]+.+')">
                [CMDI Best Practice] the CMD profile of this record can't be found in the /cmd:CMD/@xsi:schemaLocation or /cmd:CMD/cmd:Header/cmd:MdProfile. The profile should be known for the record to be processed properly in the CLARIN joint metadata domain!
            </assert>
        </rule>
    </pattern>
    <pattern>
        <title>Test for known profile</title>
        <rule role="warning" context="/cmd12:CMD">
            <assert test="matches(@xsi:schemaLocation,'clarin.eu:cr[0-9]+:p_[0-9]+.+') or matches(cmd12:Header/cmd12:MdProfile,'clarin.eu:cr[0-9]+:p_[0-9]+.+')">
                [CMDI Best Practice] the CMD profile of this record can't be found in the /cmd:CMD/@xsi:schemaLocation or /cmd:CMD/cmd:Header/cmd:MdProfile. The profile should be known for the record to be processed properly in the CLARIN joint metadata domain!
            </assert>
        </rule>
    </pattern>
    
    <!-- Is the CMD namespace bound to a schema? -->
    <pattern>
        <title>Test for CMD namespace schema binding</title>
        <rule role="warning" context="/cmd11:CMD">
            <assert test="matches(@xsi:schemaLocation,'http://www.clarin.eu/cmd/ ')">
                [possible CMDI Best Practice] is the CMD 1.1 namespace properly bound to a profile schema?
            </assert>
        </rule>
    </pattern>
    <pattern>
        <title>Test for CMD namespace schema binding</title>
        <rule role="warning" context="/cmd12:CMD">
            <assert test="matches(@xsi:schemaLocation,'http://www.clarin.eu/cmd/1 ')">
                [possible CMDI Best Practice] is the CMD 1.2 namespace properly bound to a profile schema?
            </assert>
        </rule>
    </pattern>
    
    <!-- Is the cmd:CMD root there? -->
    <pattern>
        <title>Test for cmd:CMD root</title>
        <rule role="warning" context="/">
            <assert test="exists(cmd11:CMD) or exists(cmd12:CMD)">
                [CMDI violation] is this really a CMD record? Is the namespace properly declared, e.g., including ending slash?
            </assert>
        </rule>
    </pattern>
    
</schema>
