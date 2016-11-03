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
package nl.mpi.tla.flat.deposit.sip;

import java.io.File;
import java.net.URI;
import java.util.Date;
import java.util.Set;
import nl.mpi.tla.flat.deposit.DepositException;
import org.w3c.dom.Document;

/**
 *
 * @author menzowi
 */
public interface SIPInterface {
    
    public File getBase();
    
    public Document getRecord();
    
    // PID
    public boolean hasPID();
    
    public void setPID(URI pid) throws DepositException;
    
    public URI getPID() throws DepositException;
       
    // FID
    public boolean hasFID();
    
    public void setFID(URI fid) throws DepositException;
    
    public void setFIDStream(String dsid) throws DepositException;
    
    public void setFIDasOfTimeDate(Date date) throws DepositException;
    
    public URI getFID() throws DepositException;
       
    // resources
    
    public Set<Resource> getResources();
    
    public Resource getResource(URI pid) throws DepositException;
    
    // IO
    
    public void load(File spec) throws DepositException;
    
    public void save() throws DepositException;
}
