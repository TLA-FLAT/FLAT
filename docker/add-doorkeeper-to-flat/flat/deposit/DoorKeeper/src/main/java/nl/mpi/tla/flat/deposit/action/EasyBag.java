/*
 * Copyright (C) 2016 menzowi
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

import gov.loc.repository.bagit.Bag;
import gov.loc.repository.bagit.BagFactory;
import gov.loc.repository.bagit.PreBag;
import gov.loc.repository.bagit.Manifest.Algorithm;
import gov.loc.repository.bagit.transformer.impl.UpdateCompleter;
import gov.loc.repository.bagit.writer.impl.FileSystemWriter;
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.nio.file.FileSystem;
import java.nio.file.FileSystems;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.StandardCopyOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.HashMap;
import java.util.Map;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.flat.deposit.Context;
import nl.mpi.tla.flat.deposit.DepositException;
import nl.mpi.tla.flat.deposit.Resource;
import nl.mpi.tla.flat.deposit.SIP;
import nl.mpi.tla.flat.deposit.util.Saxon;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class EasyBag extends AbstractAction {

    private static final Logger logger = LoggerFactory.getLogger(EasyBag.class.getName());

    @Override
    public boolean perform(Context context) throws DepositException {
        try {
            SIP sip = context.getSIP();
            Path fox = Paths.get(this.getParameter("foxes"));
            Path tmp = Paths.get(this.getParameter("bags"));
            Path bag = Files.createTempDirectory(tmp, "easy-bag-");
            
            // copy the resources
            for (Resource res:sip.getResources()) {
                Files.copy(res.getPath(),Paths.get(bag.toString(),res.getPath().getFileName().toString()));
            }
            
            // add a for Easy tailored version of the CMD record
            Path cmdi = Paths.get(bag.toString(),"Niet-DC-metadata/CMDI");
            Files.createDirectories(cmdi);
            XsltTransformer cmd = Saxon.buildTransformer(CreateFOX.class.getResource("/EasyBag/cmd.xsl")).load();
            cmd.setSource(new DOMSource(sip.getRecord(),sip.getBase().toURI().toString()));
            XdmDestination destination = new XdmDestination();
            cmd.setDestination(destination);
            cmd.transform();
            TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(Paths.get(cmdi.toString(),"record.xml").toFile()));

            // turn directory into a bag
            BagFactory bf = new BagFactory();
            PreBag pb = bf.createPreBag(bag.toFile());
            Bag b = pb.makeBagInPlace(BagFactory.Version.V0_97, false);                        
            b.close();

            // add Easy metadata
            Path meta = Paths.get(bag.toString(),"metadata");
            Files.createDirectory(meta);
            
            // create metadata/dataset.xml
            XsltTransformer dataset = Saxon.buildTransformer(CreateFOX.class.getResource("/EasyBag/dataset.xsl")).load();
            dataset.setSource(new StreamSource(new File(fox+"/"+sip.getFID().toString().replaceAll("[^a-zA-Z0-9]", "_")+"_CMD.xml")));
            if (this.hasParameter("creator"))
                dataset.setParameter(new QName("creator"), new XdmAtomicValue(this.getParameter("creator")));
            if (this.hasParameter("audience"))
                dataset.setParameter(new QName("audience"), new XdmAtomicValue(this.getParameter("audience")));
            if (this.hasParameter("accessRights"))
                dataset.setParameter(new QName("accessRights"), new XdmAtomicValue(this.getParameter("accessRights")));
            destination = new XdmDestination();
            dataset.setDestination(destination);
            dataset.transform();
            TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(Paths.get(meta.toString(),"dataset.xml").toFile()));
            
            // create metadata/files.xml
            XsltTransformer files = Saxon.buildTransformer(CreateFOX.class.getResource("/EasyBag/files.xsl")).load();
            files.setSource(new DOMSource(sip.getRecord(),sip.getBase().toURI().toString()));
            destination = new XdmDestination();
            files.setDestination(destination);
            files.transform();
            TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(Paths.get(meta.toString(),"files.xml").toFile()));
            
            //reload and update the bag            
            b = bf.createBag(bag.toFile(),BagFactory.Version.V0_97,BagFactory.LoadOption.BY_FILES);
            try {
                UpdateCompleter updateCompleter = new UpdateCompleter(bf);
                updateCompleter.setTagManifestAlgorithm(Algorithm.valueOfBagItAlgorithm(Algorithm.MD5.bagItAlgorithm));
                updateCompleter.setPayloadManifestAlgorithm(Algorithm.valueOfBagItAlgorithm(Algorithm.MD5.bagItAlgorithm));
                Bag newBag = updateCompleter.complete(b);
                try {
                    FileSystemWriter fsw = new FileSystemWriter(bf);
                    fsw.write(newBag, bag.toFile());
                } finally {
                    newBag.close();
                }
            } finally {
                b.close();
            }
            
            // create a zip of the bag
            Map<String, String> env = new HashMap<>();
            env.put("create", "true");     
            final FileSystem zipFileSystem = FileSystems.newFileSystem(URI.create("jar:file:" + bag.toUri().getPath().replaceFirst("/$","")+".zip"), env);
            final Path root = zipFileSystem.getPath("/");
            final String base = bag.toUri().getPath().replaceFirst("/$","").replaceFirst("/[^/]*$","");

            Files.walkFileTree(bag, new SimpleFileVisitor<Path>() {

                @Override
                public FileVisitResult visitFile(Path file,BasicFileAttributes attrs) throws IOException {
                    final Path dest = zipFileSystem.getPath(root.toString(),file.toString().replaceFirst(base,""));
                    Files.copy(file, dest, StandardCopyOption.REPLACE_EXISTING);
                    return FileVisitResult.CONTINUE;
                }

                @Override
                public FileVisitResult preVisitDirectory(Path dir,BasicFileAttributes attrs) throws IOException {
                    final Path dirToCreate = zipFileSystem.getPath(root.toString(),dir.toString().replaceFirst(base,""));
                    if (Files.notExists(dirToCreate)) {
                        Files.createDirectories(dirToCreate);
                    }
                    return FileVisitResult.CONTINUE;
                }
            });
            
            zipFileSystem.close();
            
            // delete the temporary bag directory
            Files.walkFileTree(bag, new SimpleFileVisitor<Path>() {
                
                @Override
                public FileVisitResult visitFile(Path file, BasicFileAttributes attrs) throws IOException {
                    Files.delete(file);
                    return FileVisitResult.CONTINUE;
                }

                @Override
                public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                    Files.delete(dir);
                    return FileVisitResult.CONTINUE;
                }
            });
            
            logger.info("Created Easy bag["+bag+"]");

        } catch(Exception e) {
            throw new DepositException("Creation of the Easy bag failed!",e);
        }

        return true;
    }
    
}
