/*
 * Copyright (C) 2015 menzowi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package nl.mpi.tla.flat.deposit;

import java.io.File;
import java.net.URI;
import java.nio.file.Files;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.tree.wrapper.VirtualNode;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 *
 * @author menzowi
 */
public class SIP {
    
    private static final Logger logger = LoggerFactory.getLogger(SIP.class.getName());
    
    public static String CMD_NS = "http://www.clarin.eu/cmd/";
    public static String LAT_NS = "http://lat.mpi.nl/";
    
    protected File base = null;

    protected Document rec = null;
    
    protected Set<Resource> resources = new LinkedHashSet();
    
    protected Map<String,String> namespaces = new LinkedHashMap<>();
    
    public SIP(File spec) throws DepositException {
        this.base = spec;
        load(spec);
        loadResourceList();
    }
    
    // resources
    
    private void loadResourceList() throws DepositException {
        try {
            for (XdmItem resource:Saxon.xpath(Saxon.wrapNode(this.rec),"/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']",null,getNamespaces())) {
                Node resNode = Saxon.unwrapNode((XdmNode)resource);
                Resource res = new Resource(base.toURI().resolve(Saxon.xpath2string(resource,"cmd:ResourceRef",null,getNamespaces())),resNode);
                if (Saxon.xpath2boolean(resource,"normalize-space(cmd:ResourceType/@mimetype)!=''",null,getNamespaces())) {
                    res.setMime(Saxon.xpath2string(resource,"cmd:ResourceType/@mimetype"));
                }
                if (Saxon.xpath2boolean(resource,"normalize-space(cmd:ResourceRef/@lat:localURI)!=''",null,getNamespaces())) {
                    File resFile = new File(base.toPath().getParent().normalize().toString(),Saxon.xpath2string(resource,"cmd:ResourceRef/@lat:localURI",null,getNamespaces()));
                    if (resFile.exists()) {
                        if (resFile.canRead()) {
                            res.setFile(resFile);
                        } else
                            logger.warn("local file for ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,getNamespaces())+"]["+resFile.getPath()+"] isn't readable!");
                    } else
                        logger.warn("local file for ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,getNamespaces())+"]["+resFile.getPath()+"] doesn't exist!");
                }
                if (resources.contains(res)) {
                    logger.warn("double ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,getNamespaces())+"]["+res.getURI()+"]!");
                } else {
                    resources.add(res);
                    logger.debug("ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,getNamespaces())+"]["+res.getURI()+"]");
                }
            }
        } catch(SaxonApiException e) {
            throw new DepositException(e);
        }
    }
    
    public Set<Resource> getResources() {
        return this.resources;
    }
    
    public void saveResourceList() throws DepositException {
        try {
            for (Resource res:getResources()) {
                Node node = res.getNode();
                if (res.hasMime()) {
                    Element rt = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceType", null, getNamespaces()));
                    rt.setAttribute("mimetype", res.getMime());
                }
                if (res.hasFile()) {
                    Element rr = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceRef", null, getNamespaces()));
                    rr.setAttributeNS(LAT_NS,"lat:localURI",base.getParentFile().toPath().normalize().relativize(res.getFile().toPath().normalize()).toString());
                }
            }                    
        } catch(Exception e) {
            throw new DepositException(e);
        }
    }
    
    // IO
    
    public void load(File spec) throws DepositException {
        try {
            this.rec = Saxon.buildDOM(spec);
        } catch(Exception e) {
            throw new DepositException(e);
        }
    }
    
    public void save() throws DepositException {
        try {
            File org = new File(base.toString()+".org");
            if (!org.exists())
                Files.copy(base.toPath(),org.toPath());
            saveResourceList();
            DOMSource source = new DOMSource(rec);
            Saxon.save(source,base);
        } catch(Exception e) {
            throw new DepositException(e);
        }
    }
    
    // utils
        
    public Map<String,String> getNamespaces() {
        if (this.namespaces.size()==0) {
            namespaces.put("cmd", CMD_NS);
            namespaces.put("lat", LAT_NS);
        }
        return this.namespaces;
    }
    
}
