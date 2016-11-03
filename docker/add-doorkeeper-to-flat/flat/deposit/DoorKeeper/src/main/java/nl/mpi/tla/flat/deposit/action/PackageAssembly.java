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
import java.net.URI;
import java.util.UUID;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.sip.Resource;
import org.apache.commons.io.FileUtils;
import org.apache.commons.io.FilenameUtils;
import org.apache.http.client.fluent.Request;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class PackageAssembly extends AbstractAction {

    private static final Logger logger = LoggerFactory.getLogger(PackageAssembly.class.getName());
    
    @Override
    public boolean perform(Context context) throws DepositException {
        try {
            File dir = new File(getParameter("dir","./data"));
            if (!dir.exists())
                 FileUtils.forceMkdir(dir);
            int downloads = 0;
            for (Resource res:context.getSIP().getResources()) {
                if (res.hasFile()) {
                    if (res.getFile().canRead()) {
                        logger.info("Previously download["+res.getFile()+"] of Resource["+res.getURI()+"] is still available.");
                        continue;
                    } else
                        logger.info("Previously download["+res.getFile()+"] of Resource["+res.getURI()+"] isn't available anymore!");
                }
                URI uri = res.getURI();
                if (uri.toString().startsWith(dir.getAbsoluteFile().toURI().toString())) {
                    // the file is already in the workdir resources directory
                    res.setFile(new File(uri.getSchemeSpecificPart()));
                } else if (uri.toString().startsWith("hdl:"+getParameter("prefix","foo")+"/") || uri.toString().startsWith("http://hdl.handle.net/"+getParameter("prefix","foo")+"/")) {
                    // it has already a local handle
                    // TODO: what to do? resolve to its local location, and check?
                } else if (uri.getScheme().equals("file")) {
                    // TODO: limit to specific locations
                    File src  = new File(uri.getSchemeSpecificPart());
                    File copy = new File(dir+"/"+UUID.randomUUID().toString()+"."+src.toString().replace(".*(\\.|$)",""));
                    FileUtils.copyFile(src,copy);
                    res.setFile(copy);
                } else {
                    // download the content into a local file 
                    String ext = FilenameUtils.getExtension(uri.getPath());
                    File file = dir.toPath().resolve("./"+UUID.randomUUID().toString()+(!ext.equals("")?"."+ext:"")).toFile();
                    Request.Get(uri).execute().saveContent(file);
                    res.setFile(file);
                    logger.info("Downloaded Resource["+(++downloads)+"]["+uri+"] to ["+file+"]");
                }
            }
            if (downloads>0) {
                context.getSIP().save();
            }
        } catch (Exception ex) {
            throw new DepositException("Couldn't assemble the package!",ex);
        }
        return true;
    }
    
}
