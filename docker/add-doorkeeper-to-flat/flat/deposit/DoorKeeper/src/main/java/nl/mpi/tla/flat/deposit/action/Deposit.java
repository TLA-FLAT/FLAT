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

import com.yourmediashelf.fedora.client.FedoraClient;
import static com.yourmediashelf.fedora.client.FedoraClient.*;
import com.yourmediashelf.fedora.client.FedoraCredentials;
import com.yourmediashelf.fedora.client.request.FedoraRequest;
import com.yourmediashelf.fedora.client.response.IngestResponse;
import com.yourmediashelf.fedora.client.response.GetDatastreamResponse;
import java.io.File;
import java.io.FileInputStream;
import java.security.KeyStore;
import java.util.Collection;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManagerFactory;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class Deposit extends AbstractAction {

    private static final Logger logger = LoggerFactory.getLogger(Deposit.class.getName());

    @Override
    public boolean perform(Context context) throws DepositException {
        javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(
            new javax.net.ssl.HostnameVerifier(){
                public boolean verify(String hostname,javax.net.ssl.SSLSession sslSession) {
                    return true;
                }
        });
        try {
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
//            String keystore = System.getProperty("javax.net.ssl.trustStore"); 
//            if (keystore!=null) {
//                File ks = new File(keystore);
//                if (ks.exists())
//                    logger.debug("keystore["+keystore+"]["+ks.getAbsolutePath()+"] exists");
//                else
//                    logger.error("keystore["+keystore+"]["+ks.getAbsolutePath()+"] doesn't exist!");
//            } else
//                logger.debug("keystore[NULL]");
            SIP sip = context.getSIP();
            logger.debug("Fedora Commons["+this.getParameter("fedoraServer")+"]["+this.getParameter("fedoraUser")+":"+this.getParameter("fedoraPassword")+"]");
            if (!FedoraRequest.isDefaultClientSet()) {
                FedoraCredentials credentials = new FedoraCredentials(this.getParameter("fedoraServer"), this.getParameter("fedoraUser"), this.getParameter("fedoraPassword"));
                FedoraClient fedora = new FedoraClient(theSslContext,credentials);
                fedora.debug(true);
                FedoraRequest.setDefaultClient(fedora);
            }
            logger.debug("Fedora Commons repository["+FedoraClient.describeRepository().xml(true).execute()+"]");
            
            Collection<File> foxs = FileUtils.listFiles(new File(this.getParameter("dir", "./fox")),new String[] {"xml"},true);
            logger.debug("Loading ["+foxs.size()+"] FOX files from dir["+this.getParameter("dir", "./fox")+"]");
            for (File fox:foxs) {
                logger.debug("FOX["+fox+"]");
                IngestResponse iResponse = ingest().format("info:fedora/fedora-system:FOXML-1.1").content(fox).ignoreMime(true).execute();
                logger.info("Created FedoraObject["+iResponse.getPid()+"]["+iResponse.getLocation()+"]");
            }
            for (Resource res:sip.getResources()) {
                GetDatastreamResponse dsResponse = getDatastream(res.getFID().toString(),"OBJ").execute();
                res.setFIDStream("OBJ");
                res.setFIDasOfTimeDate(dsResponse.getLastModifiedDate());
            }
            
            GetDatastreamResponse dsResponse = getDatastream(sip.getFID().toString(),"CMD").execute();
            sip.setFIDStream("CMD");
            sip.setFIDasOfTimeDate(dsResponse.getLastModifiedDate());

            sip.save();
        } catch(Exception e) {
            throw new DepositException("The actual deposit in Fedora failed!",e);
        }
        return true;
    }
    
}
