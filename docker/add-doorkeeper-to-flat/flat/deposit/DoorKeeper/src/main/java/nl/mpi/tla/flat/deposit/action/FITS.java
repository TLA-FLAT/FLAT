/*
 * Copyright (C) 2016
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
import java.net.URL;
import java.util.Set;
import net.sf.saxon.s9api.XdmNode;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.sip.Resource;
import nl.mpi.tla.flat.deposit.sip.SIPInterface;
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
    	String fitsService = getParameter("fitsService");
        if (!fitsService.endsWith("/")) {
            fitsService += "/";
        }
    	String mimetypesFileLocation = getParameter("mimetypes");
    	
    	FITSHandler fitsHandler = null;
        try {
            URL fitsURL = new URL(fitsService);
            fitsHandler = new FITSHandler(fitsURL, mimetypesFileLocation);
        } catch (Exception ex) {
            String message = "Error retrieving FITS instance";
            logger.error(message, ex);
            throw new DepositException(message, ex);
        }
    	
    	SIPInterface sip = context.getSIP();
    	Set<Resource> resources = sip.getResources();
    	for(Resource currentResource : resources) {
            if(currentResource.hasFile()) {
                File currentFile = currentResource.getFile();
    			
                XdmNode result;
                try {
                    result = fitsHandler.performFitsCheck(currentFile);
                } catch (Exception e) {
                    logger.error("Error while performing FITS typecheck for file '{}'", currentFile);
                    allAcceptable = false;
                    continue;
                }
    			
                String mimetype;
                try {
                    mimetype = fitsHandler.getResultMimetype(currentFile, result);
                } catch (Exception ex) {
                    String message = "Error retrieving FITS info for resource["+currentFile+"]";
                    logger.error(message, ex);
                    throw new DepositException(message, ex);
                }
    			
                if(!fitsHandler.isMimetypeAcceptable(mimetype)) {
                    logger.error("File '{}' has a mimetype which is NOT ALLOWED in this repository: '{}'", currentFile, mimetype);
                    allAcceptable = false;
                } else {
                    logger.info("File '{}' has a mimetype which is ALLOWED in this repository: '{}'", currentFile, mimetype);
                    if(currentResource.hasMime() && !currentResource.getMime().equals(mimetype)) {
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
