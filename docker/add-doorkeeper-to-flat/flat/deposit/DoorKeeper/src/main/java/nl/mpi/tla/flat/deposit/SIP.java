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
import java.net.URISyntaxException;
import java.nio.file.Files;
import java.nio.file.attribute.FileTime;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import javax.xml.transform.dom.DOMSource;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import nl.mpi.tla.flat.deposit.util.Global;
import static nl.mpi.tla.flat.deposit.util.Global.NAMESPACES;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.slf4j.Marker;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
 *
 * @author menzowi
 */
public class SIP {
    
    private static final Logger logger = LoggerFactory.getLogger(SIP.class.getName());
    protected Marker marker = null;
    
    public static String CMD_NS = "http://www.clarin.eu/cmd/";
    public static String LAT_NS = "http://lat.mpi.nl/";
    
    protected Node self = null;
    protected File base = null;
    protected URI pid = null;
    protected URI fid = null;

    protected Document rec = null;
    
    protected Set<Resource> resources = new LinkedHashSet();
    
    protected Map<String,String> namespaces = new LinkedHashMap<>();
    
    public SIP(File spec) throws DepositException {
        this.base = spec;
        load(spec);
        loadResourceList();
    }
    
    public File getBase() {
        return this.base;
    }
    
    public Document getRecord() {
        return this.rec;
    }
    
    // PID
    public boolean hasPID() {
        return (this.pid != null);
    }
    
    public void setPID(URI pid) throws DepositException {
        if (this.pid!=null)
            throw new DepositException("SIP["+this.base+"] has already a PID!");
        if (pid.toString().startsWith("hdl:")) {
            this.pid = pid;
        } else if (pid.toString().startsWith("http://hdl.handle.net/")) {
            try {
                this.pid = new URI(pid.toString().replace("http://hdl.handle.net/", "hdl:"));
            } catch (URISyntaxException ex) {
                throw new DepositException(ex);
            }
        } else {
            throw new DepositException("The URI["+pid+"] isn't a valid PID!");
        }
    }
    
    public URI getPID() throws DepositException {
        if (this.pid==null)
            throw new DepositException("SIP["+this.base+"] has no PID yet!");
        return this.pid;
    }
       
    // FID
    public boolean hasFID() {
        return (this.fid != null);
    }
    
    public void setFID(URI fid) throws DepositException {
        if (this.fid!=null)
            throw new DepositException("SIP["+this.base+"] has already a Fedora Commons PID!");
        if (fid.toString().startsWith("lat:")) {
            this.fid = fid;
        } else {
            throw new DepositException("The URI["+fid+"] isn't a valid FLAT Fedora Commons PID!");
        }
    }
    
    public void setFIDStream(String dsid) throws DepositException {
        if (this.fid==null)
            throw new DepositException("SIP["+this.base+"] has no Fedora Commons PID yet!");
        try {
            this.fid = new URI(this.fid.toString()+"#"+dsid);
        } catch (URISyntaxException ex) {
           throw new DepositException(ex);
        }
    }
    
    public void setFIDasOfTimeDate(Date date) throws DepositException {
        if (this.fid==null)
            throw new DepositException("SIP["+this.base+"] has no Fedora Commons PID yet!");
        try {
            this.fid = new URI(this.fid.toString()+"@"+Global.asOfDateTime(date));
        } catch (URISyntaxException ex) {
           throw new DepositException(ex);
        }
    }
    
    public URI getFID() throws DepositException {
        if (this.fid==null)
            throw new DepositException("SIP["+this.base+"] has no Fedora Commons PID yet!");
        return this.fid;
    }
       
    // resources
    
    private void loadResourceList() throws DepositException {
        try {
            for (XdmItem resource:Saxon.xpath(Saxon.wrapNode(this.rec),"/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']",null,NAMESPACES)) {
                Node resNode = Saxon.unwrapNode((XdmNode)resource);
                Resource res = new Resource(base.toURI().resolve(Saxon.xpath2string(resource,"cmd:ResourceRef",null,NAMESPACES)),resNode);
                if (Saxon.xpath2boolean(resource,"normalize-space(cmd:ResourceType/@mimetype)!=''",null,NAMESPACES)) {
                    res.setMime(Saxon.xpath2string(resource,"cmd:ResourceType/@mimetype",null,NAMESPACES));
                }
                if (Saxon.xpath2boolean(resource,"normalize-space(cmd:ResourceRef/@lat:localURI)!=''",null,NAMESPACES)) {
                    File resFile = new File(base.toPath().getParent().normalize().toString(),Saxon.xpath2string(resource,"cmd:ResourceRef/@lat:localURI",null,NAMESPACES));
                    if (resFile.exists()) {
                        if (resFile.canRead()) {
                            res.setFile(resFile);
                        } else
                            logger.warn("local file for ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,NAMESPACES)+"]["+resFile.getPath()+"] isn't readable!");
                    } else
                        logger.warn("local file for ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,NAMESPACES)+"]["+resFile.getPath()+"] doesn't exist!");
                }
                if (Saxon.xpath2boolean(resource,"normalize-space(cmd:ResourceRef/@lat:flatURI)!=''",null,NAMESPACES)) {
                    res.setFID(new URI(Saxon.xpath2string(resource,"cmd:ResourceRef/@lat:flatURI",null,NAMESPACES)));
                }
                if (resources.contains(res)) {
                    logger.warn("double ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,NAMESPACES)+"]["+res.getURI()+"]!");
                } else {
                    resources.add(res);
                    logger.debug("ResourceProxy["+Saxon.xpath2string(resource,"cmd:ResourceRef",null,NAMESPACES)+"]["+res.getURI()+"]");
                }
            }
        } catch(SaxonApiException e) {
            throw new DepositException(e);
        } catch (URISyntaxException e) {
            throw new DepositException(e);
        }
    }
    
    public Set<Resource> getResources() {
        return this.resources;
    }
    
    public Resource getResource(URI pid) throws DepositException {
        for (Resource res:getResources()) {
            if (res.getPID().equals(pid)) {
                return res;
            }
        }
        throw new DepositException("SIP["+this.base+"] has no Resource with this PID["+pid+"]!");
    }
    
    public void saveResourceList() throws DepositException {
        try {
            for (Resource res:getResources()) {
                Node node = res.getNode();
                if (res.hasMime()) {
                    Element rt = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceType", null, NAMESPACES));
                    rt.setAttribute("mimetype", res.getMime());
                }
                if (res.hasFile()) {
                    Element rr = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceRef", null, NAMESPACES));
                    rr.setAttribute("lat:localURI",base.getParentFile().toPath().normalize().relativize(res.getFile().toPath().normalize()).toString());
                }
                if (res.hasPID()) {
                    Element rr = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceRef", null, NAMESPACES));
                    rr.setTextContent(res.getPID().toString());
                }
                if (res.hasFID()) {
                    Element rr = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(node), "cmd:ResourceRef", null, NAMESPACES));
                    rr.setAttribute("lat:flatURI",res.getFID().toString());
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

            Element cmd = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(this.rec), "/cmd:CMD", null, NAMESPACES));
            cmd.setAttributeNS("http://www.w3.org/2000/xmlns/", "xmlns:lat", LAT_NS);
            
            String self = Saxon.xpath2string(Saxon.wrapNode(this.rec), "/cmd:CMD/cmd:Header/cmd:MdSelfLink", null, NAMESPACES);
            if (!self.trim().equals("")) {
                try {
                    this.setPID(new URI(self));
                } catch(DepositException e) {
                    logger.warn("current MdSelfLink["+self+"] isn't a valid PID, ignored for now!");
                }
            }
            String flat = Saxon.xpath2string(Saxon.wrapNode(this.rec), "/cmd:CMD/cmd:Header/cmd:MdSelfLink/@lat:flatURI", null, NAMESPACES);
            if (!flat.trim().equals("")) {
                try {
                    this.setFID(new URI(flat));
                } catch(DepositException e) {
                    logger.warn("current MdSelfLink/@lat:flatURI["+flat+"] isn't a valid Fecora Commons PID, ignored for now!");
                }
            }
        } catch(Exception e) {
            throw new DepositException(e);
        }
    }
    
    public void save() throws DepositException {
        try {
            if (base.exists()) {
                // always keep the org around
                File org = new File(base.toString()+".org");
                if (!org.exists())
                    Files.copy(base.toPath(),org.toPath());
                // and keep timestamped backups
                FileTime stamp = Files.getLastModifiedTime(base.toPath());
                SimpleDateFormat df = new SimpleDateFormat("yyyyMMdd-HHmmss");
                String ext = df.format(stamp.toMillis());
                int i = 0;
                File bak = new File(base.toString()+"."+ext);
                while (bak.exists())
                    bak = new File(base.toString()+"."+ext+"."+(++i));
                Files.move(base.toPath(),bak.toPath());
            }
            // put PID into place
            if (this.hasPID()) {
                Element self = null;
                XdmItem _self = Saxon.xpathSingle(Saxon.wrapNode(this.rec),"/cmd:CMD/cmd:Header/cmd:MdSelfLink",null,NAMESPACES);
                if (_self==null) {
                    Element profile = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(this.rec), "/cmd:CMD/cmd:Header/cmd:MdProfile", null, NAMESPACES));
                    Element header = (Element)Saxon.unwrapNode((XdmNode)Saxon.xpath(Saxon.wrapNode(this.rec), "/cmd:CMD/cmd:Header", null, NAMESPACES));
                    self = rec.createElementNS(CMD_NS, "MdSelfLink");
                    self.setTextContent(this.getPID().toString());
                    header.insertBefore(self, profile);
                } else {
                    self = (Element)Saxon.unwrapNode((XdmNode)_self);
                }
                if (this.hasFID()) {
                    self.setAttribute("lat:flatURI",this.getFID().toString());
                }
            }   
            // save changes to the resource list
            saveResourceList();
            DOMSource source = new DOMSource(rec);
            Saxon.save(source,base);
        } catch(Exception e) {
            throw new DepositException(e);
        }
    }
}
