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
import java.io.FileInputStream;
import java.net.URI;
import java.security.KeyStore;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;
import nl.knaw.meertens.pid.PIDService;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import org.apache.commons.configuration.XMLConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class EPICHandleCreation extends AbstractAction {
    
    private static final Logger logger = LoggerFactory.getLogger(EPICHandleCreation.class.getName());

    @Override
    public boolean perform(Context context) throws DepositException {
        
        String fedora = this.getParameter("fedoraServer");
        String epic   = this.getParameter("epicConfig");

        if (epic == null) {
            logger.error("No EPIC configuration has been specified! Use the epicConfig parameter.");
            return false;
        }
        
        File config = new File(epic);
        if (!config.exists()) {
            logger.error("The EPIC configuration["+epic+"] doesn't exist!");
            return false;
        } else if (!config.isFile()) {
            logger.error("The EPIC configuration["+epic+"] isn't a file!");
            return false;
        } else if (!config.canRead()) {
            logger.error("The EPIC configuration["+epic+"] can't be read!");
            return false;
        }

        try {
            
            javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(
                new javax.net.ssl.HostnameVerifier(){
                    public boolean verify(String hostname,javax.net.ssl.SSLSession sslSession) {
                        return true;
                    }
                });
            
            SSLContext theSslContext = null;
            if (this.hasParameter("trustStore")) {
                String ts = this.getParameter("trustStore");
                String tsPass = this.getParameter("trustStorePass");
                KeyStore theClientTruststore = KeyStore.getInstance(KeyStore.getDefaultType());
                theClientTruststore.load(new FileInputStream(ts),tsPass.toCharArray());
                TrustManagerFactory theTrustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
                theTrustManagerFactory.init(theClientTruststore);
                theSslContext = SSLContext.getInstance("TLS");
                theSslContext.init(null,theTrustManagerFactory.getTrustManagers(),null);
                HttpsURLConnection.setDefaultSSLSocketFactory(theSslContext.getSocketFactory());
            }
            
            logger.debug("EPIC configuration["+config.getAbsolutePath()+"]");
            PIDService ps = new PIDService(new XMLConfiguration(config), theSslContext);

            String fid = context.getSIP().getFID().toString().replaceAll("#.*","");
            String dsid = context.getSIP().getFID().getRawFragment().replaceAll("@.*","");
            String asof = context.getSIP().getFID().getRawFragment().replaceAll(".*@","");

            URI    pid  = context.getSIP().getPID();
            String uuid = pid.toString().replaceAll(".*/","");
            String loc  = fedora+"/objects/"+fid+"/datastreams/"+dsid+"/content?asOfDateTime="+asof;
            
            logger.info("Create handle["+pid+"]["+uuid+"] -> URI["+loc+"]");
            
            String hdl = ps.requestHandle(uuid, loc);

            logger.info("Created handle["+hdl+"] -> URI["+loc+"]");

            for (Resource res:context.getSIP().getResources()) {
                fid = res.getFID().toString().replaceAll("#.*","");
                dsid = res.getFID().getRawFragment().replaceAll("@.*","");
                asof = res.getFID().getRawFragment().replaceAll(".*@","");

                pid  = res.getPID();
                uuid = pid.toString().replaceAll(".*/","");
                loc  = fedora+"/objects/"+fid+"/datastreams/"+dsid+"/content?asOfDateTime="+asof;

                logger.info("Create handle["+pid+"]["+uuid+"] -> URI["+loc+"]");

                hdl = ps.requestHandle(uuid, loc);

                logger.info("Created handle["+hdl+"] -> URI["+loc+"]");
            }
        } catch(Exception e) {
            throw new DepositException(e);
        }
        
        return true;
    }
    
}
