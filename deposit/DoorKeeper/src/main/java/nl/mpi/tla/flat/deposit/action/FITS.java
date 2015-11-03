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
package nl.mpi.tla.flat.deposit.action;

import java.io.File;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import net.sf.saxon.s9api.XdmValue;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.util.TypeCheckHandler;

/**
 *
 * @author menzowi
 */
public class FITS extends AbstractAction {
	
	private static final Logger logger = LoggerFactory.getLogger(FITS.class);
    
	
	//TODO to be initialized - using Spring?
	private TypeCheckHandler typecheck;
	
    @Override
    public boolean perform(Context context) throws DepositException {
    	
    	boolean allAcceptable = true;
    	
    	SIP sip = context.getSIP();
    	Set<Resource> resources = sip.getResources();
    	for(Resource currentResource : resources) {
    		if(currentResource.hasFile()) {
    			File currentFile = currentResource.getFile();
    			if(!typecheck.isFileAcceptable(currentFile)) {
    				allAcceptable = false;
    				logger.warn("File '{}' not acceptable", currentFile);
    			}
    		}
    	}
    	
    	return allAcceptable;
    }
    
    
    //TODO use Spring to inject the field instead
    public void setTypeCheckHandler(TypeCheckHandler typecheck) {
    	this.typecheck = typecheck;
    }
}
