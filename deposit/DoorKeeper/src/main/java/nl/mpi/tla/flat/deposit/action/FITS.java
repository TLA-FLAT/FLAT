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
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.harvard.hul.ois.fits.FitsOutput;
import edu.harvard.hul.ois.fits.exceptions.FitsConfigurationException;
import edu.harvard.hul.ois.fits.exceptions.FitsException;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.action.fits.util.FITSHandler;

/**
 *
 * @author menzowi
 */
public class FITS extends AbstractAction {
	
	private static final Logger logger = LoggerFactory.getLogger(FITS.class);
    
	
    @Override
    public boolean perform(Context context) throws DepositException {
    	
    	boolean allAcceptable = true;
    	
    	//TODO What default to use?
    	String fitsHome = getParameter("fits_home", null);
    	String mimetypesFileLocation = getParameter("mimetypes", null);
    	
    	FITSHandler fitsHandler = null;
		try {
			fitsHandler = FITSHandler.getNewFITSHandler(fitsHome, mimetypesFileLocation);
		} catch (FitsConfigurationException ex) {
			String message = "Error retrieving FITS instance";
			logger.error(message, ex);
			throw new DepositException(message, ex);
		}
    	
    	SIP sip = context.getSIP();
    	Set<Resource> resources = sip.getResources();
    	for(Resource currentResource : resources) {
    		if(currentResource.hasFile()) {
    			File currentFile = currentResource.getFile();
    			
    			FitsOutput result = null;
    			try {
					result = fitsHandler.performFitsCheck(currentFile);
				} catch (FitsException e) {
					logger.error("Error while performing FITS typecheck for file '{}'", currentFile);
					allAcceptable = false;
					continue;
				}
    			
    			String mimetype = fitsHandler.getResultMimetype(currentFile, result);
    			
    			if(!fitsHandler.isMimetypeAcceptable(mimetype)) {
    				logger.error("File '{}' has a mimetype which is NOT ALLOWED in this repository: '{}'", currentFile, mimetype);
    				allAcceptable = false;
    			} else {
    				logger.debug("File '{}' has a mimetype which is ALLOWED in this repository: '{}'", currentFile, mimetype);
    				if(currentResource.hasMime()) {
    					logger.warn("Resource mimetype changed from '{}' to '{}'", currentResource.getMime(), mimetype);
    				}
    				logger.debug("Setting resource mimetype to '{}'", mimetype);
    				currentResource.setMime(mimetype);
    			}
    		}
    	}
    	
    	return allAcceptable;
    }
}
