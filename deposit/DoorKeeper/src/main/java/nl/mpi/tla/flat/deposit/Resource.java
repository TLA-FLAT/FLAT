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
import org.w3c.dom.Node;

/**
 *
 * @author menzowi
 */
public class Resource {
    
    protected Node node = null;
    protected URI uri = null;
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
    
    public void setMime(String mime) {
        this.mime = mime;
    }
    
    public boolean hasMime() {
        return (this.mime!=null);
    }
    
    public String getMime() {
        return this.mime;
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
