/*
 * Copyright (C) 2014 The Language Archive - Max Planck Institute for Psycholinguistics
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
package nl.mpi.tla.lat2fox;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Collection;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.schemanon.Message;
import nl.mpi.tla.schemanon.SchemAnon;
import nl.mpi.tla.schemanon.SaxonUtils;
import org.apache.commons.io.FileUtils;

/**
 * @author Menzo Windhouwer
 */
public class Main {
    
    private static void showHelp() {
        System.err.println("INF: lat2fox <options> -- <DIR>");
        System.err.println("INF: <DIR>     source directory to recurse for CMD files");
        System.err.println("INF: lat2fox options:");
        System.err.println("INF: -f=<DIR>  directory to store the FOX files (optional)");
        System.err.println("INF: -i=<DIR>  replace source <DIR> by this <DIR> in the FOX files (optional)");
    }

    public static void main(String[] args) {
        String dir = ".";
        String fdir = null;
        String idir = null;
        // check command line
        OptionParser parser = new OptionParser( "f:i:?*" );
        OptionSet options = parser.parse(args);
        if (options.has("f"))
            fdir = (String)options.valueOf("f");
        if (options.has("i"))
            idir = (String)options.valueOf("i");
        if (options.has("?")) {
            showHelp();
            System.exit(0);
        }
        
        List arg = options.nonOptionArguments();
        if (arg.size()!=1) {
            System.err.println("FTL: missing the source <DIR> argument!");
            showHelp();
            System.exit(1);
        }
        dir = (String)arg.get(0);

        try {
            SaxonExtensionFunctions.registerAll(SaxonUtils.getProcessor().getUnderlyingConfiguration());
        } catch (Exception e) {
            System.err.println("ERR: couldn't register the Saxon extension functions: "+e);
            e.printStackTrace();
        }
        try {
            if (fdir == null)
                fdir = dir + "/fox";
            // create lookup document for relations
            XsltTransformer rels = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2rels.xsl")).load();
            rels.setParameter(new QName("dir"), new XdmAtomicValue("file:"+dir));
            rels.setSource(new StreamSource(Main.class.getResource("/null.xml").toString()));
            XdmDestination destination = new XdmDestination();
            rels.setDestination(destination);
            rels.transform();
            File map = new File(dir+"/relations.xml");
            TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(map));
            // CMDI 2 FOX
            // create the fox dir
            FileUtils.forceMkdir(new File(fdir));
            Collection<File> inputs = FileUtils.listFiles(new File(dir),new String[] {"cmdi"},true);
            XsltExecutable cmd2fox = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2fox.xsl"));
            int i = 0;
            for (File input:inputs) {
                if (!input.isHidden() && !input.getAbsolutePath().matches(".*/(corpman|sessions)/.*")) {
                    XsltTransformer fox = cmd2fox.load();
                    fox.setParameter(new QName("rels-uri"), new XdmAtomicValue("file:"+map.getAbsolutePath()));
                    fox.setParameter(new QName("conversion-base"), new XdmAtomicValue(dir));
                    if (idir != null)
                        fox.setParameter(new QName("import-base"), new XdmAtomicValue(idir));
                    fox.setParameter(new QName("fox-base"), new XdmAtomicValue(fdir));
                    fox.setSource(new StreamSource(input));
                    destination = new XdmDestination();
                    fox.setDestination(destination);
                    fox.transform();
                    File out = new File(fdir + "/lat-"+(++i)+".xml");
                    TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(out));
                    System.err.println("DBG: created["+out.getAbsolutePath()+"]");
                }
            }
        } catch(Exception ex) {
            System.err.println("FATAL: "+ex);
            ex.printStackTrace(System.err);
        }
    }
}
