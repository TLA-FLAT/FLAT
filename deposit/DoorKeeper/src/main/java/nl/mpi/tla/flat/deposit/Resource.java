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
import java.nio.file.Path;
import java.util.Date;
import nl.mpi.tla.flat.deposit.util.Global;
import org.w3c.dom.Node;

/**
 *
 * @author menzowi
 */
public class Resource {
    
    protected Node node = null;
    protected URI uri = null;
    protected URI pid = null;
    protected URI fid = null;
    protected File file = null;
    protected String mime = null;
    
    public Resource(URI uri,Node node) {
        this.uri = uri;
        this.node = node;
    }
    
    public URI getURI() {
        return this.uri;
    }
    
    public Node getNode() {
        return this.node;
    }
    
    public void setFile(File file) {
        this.file = file;
    }
    
    public boolean hasFile() {
        return (this.file!=null);
    }
    
    public File getFile() {
        return this.file;
    }
    
    public Path getPath() {
        return this.file.toPath();
    }
    
    public void setMime(String mime) {
        this.mime = mime;
    }
    
    public boolean hasMime() {
        return (this.mime!=null);
    }
    
    public String getMime() {
        if (hasMime())
            return this.mime;
        return "application/octet-stream";
    }
    
    // PID
    public boolean hasPID() {
        return (this.pid != null);
    }
    
    public void setPID(URI pid) throws DepositException {
        if (this.pid!=null)
            throw new DepositException("Resource["+this.uri+"] has already a PID!");
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
            throw new DepositException("Resource["+this.uri+"] has no PID yet!");
        return this.pid;
    }
    
    // FID
    public boolean hasFID() {
        return (this.fid != null);
    }
    
    public void setFID(URI fid) throws DepositException {
        if (this.fid!=null)
            throw new DepositException("Resource["+this.uri+"] has already a Fedora Commons PID!");
        if (fid.toString().startsWith("lat:")) {
            this.fid = fid;
        } else {
            throw new DepositException("The Resource["+fid+"] isn't a valid FLAT Fedora Commons PID!");
        }
    }
    
    public void setFIDStream(String dsid) throws DepositException {
        if (this.fid==null)
            throw new DepositException("Resource["+this.uri+"] has no Fedora Commons PID yet!");
        try {
            this.fid = new URI(this.fid.toString()+"#"+dsid);
        } catch (URISyntaxException ex) {
           throw new DepositException(ex);
        }
    }
    
    public void setFIDasOfTimeDate(Date date) throws DepositException {
        if (this.fid==null)
            throw new DepositException("Resource["+this.uri+"] has no Fedora Commons PID yet!");
        try {
            this.fid = new URI(this.fid.toString()+"@"+Global.asOfDateTime(date));
        } catch (URISyntaxException ex) {
           throw new DepositException(ex);
        }
    }
    
    public URI getFID() throws DepositException {
        if (this.fid==null)
            throw new DepositException("Resource["+this.uri+"] has no Fedora Commons PID yet!");
        return this.fid;
    }
       
    @Override
    public boolean equals(Object other) {
        if (other == null)
            return false;
        if (other == this)
            return true;
        if (!(other instanceof Resource))
            return false;
        Resource otherResource = (Resource)other;
        return otherResource.uri.equals(this.uri);
    }
    
    @Override
    public int hashCode() {
        return this.uri.hashCode();
    }
    
}
