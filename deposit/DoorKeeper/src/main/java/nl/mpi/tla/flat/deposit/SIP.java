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
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import javax.xml.transform.Source;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class SIP {
    
    private static final Logger logger = LoggerFactory.getLogger(SIP.class.getName());
    
    protected URI base = null;

    protected XdmNode rec = null;
    
    protected Set<URI> resources = new LinkedHashSet();
    
    protected Map<String,String> namespaces = new LinkedHashMap<>();
    
    public SIP(File spec) throws DepositException {
        this(spec.toURI());
    }
    
    public SIP(URI spec) throws DepositException {
        this(spec,new StreamSource(spec.toString()));
    }
    
    protected SIP(URI base,Source rec) throws DepositException {
        try {
            this.base = base;
            this.rec = Saxon.buildDocument(rec);
            loadResourceList();
        } catch(SaxonApiException e) {
            throw new DepositException(e);
        }
    }
    
    private void loadResourceList() throws SaxonApiException {
        for (XdmItem resource:Saxon.xpath(this.rec,"/cmd:CMD/cmd:Resources/cmd:ResourceProxyList/cmd:ResourceProxy[cmd:ResourceType='Resource']/cmd:ResourceRef",null,getNamespaces())) {
            URI res = base.resolve(resource.getStringValue());
            if (resources.contains(res)) {
                logger.warn("double ResourceProxy["+resource.getStringValue()+"]["+res+"]!");
            } else {
                resources.add(res);
                logger.debug("ResourceProxy["+resource.getStringValue()+"]["+res+"]");
            }
        }
    }
        
    public Map<String,String> getNamespaces() {
        if (this.namespaces.size()==0)
            namespaces.put("cmd", "http://www.clarin.eu/cmd/");
        return this.namespaces;
    }
    
}
