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
import java.io.File;
import java.util.Collection;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.apache.commons.io.FileUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class CreateFOX extends AbstractAction {

    private static final Logger logger = LoggerFactory.getLogger(CreateFOX.class.getName());

    @Override
    public boolean perform(Context context) throws DepositException {
        try {
            File dir = new File(getParameter("dir","./fox"));
            if (!dir.exists())
                 FileUtils.forceMkdir(dir);
            File xsl = new File(getParameter("cmd2fox"));
            XsltExecutable cmd2fox = null;
            if (hasParameter("cmd2dc")) {
                XsltTransformer inclCMD2DC = Saxon.buildTransformer(CreateFOX.class.getResource("/inclCMD2DC.xsl")).load();
                inclCMD2DC.setSource(new StreamSource(xsl));
                inclCMD2DC.setParameter(new QName("cmd2dc"),new XdmAtomicValue("file://"+(new File(getParameter("cmd2dc"))).getAbsolutePath()));
                XdmDestination destination = new XdmDestination();
                inclCMD2DC.setDestination(destination);
                inclCMD2DC.transform();
                cmd2fox = Saxon.buildTransformer(destination.getXdmNode());                
            } else {
                cmd2fox = Saxon.buildTransformer(xsl);
            }
            XsltTransformer fox = cmd2fox.load();
            fox.setParameter(new QName("fox-base"), new XdmAtomicValue(dir.toString()));
            fox.setParameter(new QName("rels-uri"), new XdmAtomicValue(getParameter("relations")));
            fox.setSource(new DOMSource(context.getSIP().getRecord(),context.getSIP().getBase().toURI().toString()));
            XdmDestination destination = new XdmDestination();
            fox.setDestination(destination);
            fox.transform();
            String fid = Saxon.xpath2string(destination.getXdmNode(),"/*/@PID");
            File out = new File(dir + "/"+fid.replaceAll("[^a-zA-Z0-9]", "_")+".xml");
            if (out.exists()) {
                // create a backup of the previous run
            }
            TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(out));
            logger.info("created FOX["+out.getAbsolutePath()+"]");

        } catch(Exception e) {
            throw new DepositException("The creation of FOX files failed!",e);
        }
        return true;
    }
    
}
