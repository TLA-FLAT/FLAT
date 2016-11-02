package nl.knaw.meertens.pid;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import javax.net.ssl.SSLContext;

import net.sf.json.JSONException;
import net.sf.json.JSONArray;
import net.sf.json.JSONObject;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.XMLConfiguration;
import org.apache.commons.httpclient.HostConfiguration;
import org.apache.commons.httpclient.HttpClient;
import org.apache.commons.httpclient.HttpStatus;
import org.apache.commons.httpclient.NameValuePair;
import org.apache.commons.httpclient.URI;
import org.apache.commons.httpclient.UsernamePasswordCredentials;
import org.apache.commons.httpclient.auth.AuthScope;
//import org.apache.commons.httpclient.contrib.ssl.EasySSLProtocolSocketFactory;
import org.apache.commons.httpclient.methods.GetMethod;
import org.apache.commons.httpclient.methods.PutMethod;
import org.apache.commons.httpclient.methods.StringRequestEntity;
import org.apache.commons.httpclient.protocol.Protocol;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class PIDService {
    
    private static final Logger logger = LoggerFactory.getLogger(PIDService.class.getName());
    
    private final String hostName;
    private final String host;
    private final String handlePrefix;
    private final String userName;
    private final String password;
    private final String email;
    private boolean isTest = true;
    
    private final SSLContext ssl;
	
    public PIDService(SSLContext ssl) throws ConfigurationException{
        this(new XMLConfiguration("config.xml"), ssl);
    }
	
    public PIDService(XMLConfiguration config, SSLContext ssl) throws ConfigurationException{	
        this.ssl = ssl;
        
        if( config == null)
            throw new IllegalArgumentException("No EPIC configuration specified!");
        
        for(Iterator iter =config.getKeys();iter.hasNext();) {
            logger.debug("EPIC configuration key["+iter.next()+"]");
        }

        // do something with config
        this.hostName = config.getString("hostName");
        this.host = config.getString("URI");
        this.handlePrefix = config.getString("HandlePrefix");
        this.userName = config.getString("userName");
        this.password = config.getString("password");
        this.email = config.getString("email");
        this.isTest = config.getString("status") != null && config.getString("status").equals("test");
        
        logger.debug((this.isTest?"test":"production")+" PIDService ["+this.host+"]["+this.handlePrefix+"]["+this.userName+":"+this.password+"]["+this.email+"]");
    }
	
    public String requestHandle(String a_location) throws IOException, HandleCreationException{
        return requestHandle(UUID.randomUUID().toString(), a_location);
    }
    
    public String requestHandle(String uuid,String a_location) throws IOException, HandleCreationException{
		
        if (isTest) {
            logger.info("[TESTMODE] Created Handle=["+"PIDManager_"+ a_location+"] for location["+a_location+"]");
            return "PIDManager_"+ a_location;
        }
		
        Protocol easyhttps = null;
        try {
            easyhttps = new Protocol("https", new EasySSLProtocolSocketFactory(ssl), 443);
        } catch (Exception e){
            logger.error("Problem configurating connection",e);
            throw new IOException("Problem configurating connection");
        }
        String handle = this.handlePrefix + "/" + uuid;
        logger.info("Requesting handle: " + handle);
        URI uri = new URI(host + handle, true);
		
        HttpClient client = new HttpClient();
        client.getState().setCredentials(
            new AuthScope(this.hostName, 443, "realm"),
            new UsernamePasswordCredentials(this.userName, this.password));
        client.getParams().setAuthenticationPreemptive(true);
        PutMethod httpput = new PutMethod( uri.getPathQuery());
        httpput.setRequestHeader("Content-Type", "application/json");
        HostConfiguration hc = new HostConfiguration();
        hc.setHost(uri.getHost(), uri.getPort(), easyhttps);
        httpput.setDoAuthentication(true);
						
        Map<String, Object> map = new HashMap<String, Object>();
        map.put("idx", "1");
        map.put("type", "URL");
        map.put("parsed_data",a_location);
        map.put( "timestamp", "" + System.currentTimeMillis());
        map.put("refs","");
        Map<String, Object> map2 = new HashMap<String, Object>();
        map2.put("idx", "2");
        map2.put("type", "EMAIL");
        map2.put("parsed_data",this.email);
        map2.put( "timestamp", System.currentTimeMillis());
        map2.put("refs","");
        String jsonStr = null;
        try {
            List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
            list.add(map);
            list.add(map2);
            JSONArray a = JSONArray.fromObject(list);
            jsonStr = a.toString();
            logger.info(jsonStr);
        } catch (JSONException e) {
            logger.error("Unable to create JSON Request object",e);
            throw new IOException( "Unable to create JSON Request object");
        }
		
        httpput.setRequestEntity(new StringRequestEntity( jsonStr, "application/json","UTF-8"));
				
        try {
            client.executeMethod(hc, httpput);
            if (httpput.getStatusCode() != HttpStatus.SC_CREATED ) {
                logger.error("EPIC unexpected result[" + httpput.getStatusLine().toString()+"]");
                throw new HandleCreationException("Handle creation failed. Unexpected failure: " + httpput.getStatusLine().toString() + ". " + httpput.getResponseBodyAsString());
            }
	} finally {
            logger.debug("EPIC result["+httpput.getResponseBodyAsString()+"]");
            httpput.releaseConnection();
	}
        
        //A resolvable handle is returned using the global resolver
        logger.info( "Created handle["+handle+"] for location ["+a_location+"]");
		
        return handle;
    }
	
    public void updateLocation( String a_handle, String a_location)throws IOException, HandleCreationException{
        if (isTest) {
            logger.debug("[TESTMODE] Handled request location change for Handle=["+a_handle+"] to new location["+a_location+"] ... did nothing");
            return;
        }
        UUID uuid = UUID.randomUUID();
        Protocol easyhttps = null;
        try {
            easyhttps = new Protocol("https", new EasySSLProtocolSocketFactory(), 443);
	} catch(Exception e){
            logger.error("Problem configurating connection",e);
            throw new IOException("Problem configurating connection");
        }
		
        URI uri = new URI(host + a_handle, true);		
		
        HttpClient client = new HttpClient();
		
        client.getState().setCredentials(
            new AuthScope(this.hostName, 443, "realm"),
            new UsernamePasswordCredentials(this.userName, this.password));
        client.getParams().setAuthenticationPreemptive(true);
        PutMethod httpput = new PutMethod( uri.getPathQuery());
        httpput.setRequestHeader("Content-Type", "application/json");
        HostConfiguration hc = new HostConfiguration();
        hc.setHost(uri.getHost(), uri.getPort(), easyhttps);
        httpput.setDoAuthentication(true);
	
        Map<String, Object> map = new HashMap<String, Object>();
        map.put("idx", "1");
        map.put("type", "URL");
        map.put("parsed_data",a_location);
        map.put( "timestamp", "" + System.currentTimeMillis());
        map.put("refs","");
        Map<String, Object> map2 = new HashMap<String, Object>();
        map2.put("idx", "2");
        map2.put("type", "EMAIL");
        map2.put("parsed_data",this.email);
        map2.put( "timestamp", System.currentTimeMillis());
        map2.put("refs","");
        String jsonStr = null;
        try{
            List<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
            list.add(map);
            list.add(map2);
            JSONArray a = JSONArray.fromObject(list);
            jsonStr = a.toString();
            logger.info(jsonStr);
        }
        catch( JSONException e){
            logger.error("Unable to create JSON Request object",e);
            throw new IOException("Unable to create JSON Request object");
        }
		
        //System.out.println( jsonStr);
        httpput.setRequestEntity(new StringRequestEntity( jsonStr, "application/json","UTF-8"));
				
        try {
            client.executeMethod(hc, httpput);
            if (httpput.getStatusCode() == HttpStatus.SC_NO_CONTENT) {
                logger.info( "EPIC updated handle["+a_handle+"] for location ["+a_location+"]");
            } else {
                logger.error("EPIC unexpected result[" + httpput.getStatusLine().toString()+"]");
                throw new HandleCreationException("Handle creation failed. Unexpected failure: " + httpput.getStatusLine().toString() + ". " + httpput.getResponseBodyAsString());
            }
	} finally {
            logger.debug("EPIC result["+httpput.getResponseBodyAsString()+"]");
            httpput.releaseConnection();
	}
    }
	
    public String getPIDLocation( String a_handle) throws IOException{
	Protocol easyhttps = null;
	try {
            easyhttps = new Protocol("https", new EasySSLProtocolSocketFactory(), 443);
	} catch (Exception e){
            logger.error("Problem configurating connection",e);
            throw new IOException("Problem configurating connection");
        }
        URI uri = new URI(host + a_handle, true);
		
        HttpClient client = new HttpClient();
        client.getState().setCredentials(
            new AuthScope(this.hostName, 443, "realm"),
            new UsernamePasswordCredentials(this.userName, this.password));
        client.getParams().setAuthenticationPreemptive(true);
        GetMethod httpGet = new GetMethod(uri.getPathQuery());
        httpGet.setFollowRedirects(false);
        httpGet.setQueryString(new NameValuePair[] { 
            new NameValuePair("redirect", "no") 
        }); 
        httpGet.setRequestHeader("Accept", "application/json");
        HostConfiguration hc = new HostConfiguration();
        hc.setHost(uri.getHost(), uri.getPort(), easyhttps);
        httpGet.setDoAuthentication(true);
        JSONObject json = null;
        try {
            client.executeMethod(hc, httpGet);
            if (httpGet.getStatusCode() == HttpStatus.SC_OK) {
                logger.debug(httpGet.getResponseBodyAsString());
                JSONArray jsonArr = JSONArray.fromObject(httpGet.getResponseBodyAsString());
                json = jsonArr.getJSONObject(0);
            } else {
                logger.error("EPIC unexpected result[" + httpGet.getStatusLine().toString()+"]");
                throw new IOException("Handle retrieval failed["+a_handle+"]. Unexpected failure: " + httpGet.getStatusLine().toString() + ". " + httpGet.getResponseBodyAsString());
            }
        } finally {
            logger.debug("EPIC result["+httpGet.getResponseBodyAsString()+"]");
            httpGet.releaseConnection();
        }
        String location = json.getString("parsed_data");
        return location;		
    }
	
    public URL makeActionable( String a_PID){
        URL url = null;
        try {
            url = new URL( "http://hdl.handle.net/" + a_PID);
        } catch (MalformedURLException e) {
            logger.error("couldn't make PID actionable",e);
            //do nothing
            //null will be returned
        }
        return url;
    }
}
